/* Copyright (c) 2014, Pollard Banknote Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @todo This file would benefit from a "file" class that is closer to the
 *  system calls
 */
#include <string>
#include <cstdlib>
#include <cstdio>

#ifdef _POSIX_C_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "fileutils.h"

namespace
{
// append contents of in to out. Both file descriptors must be valid
bool copy_contents(
	int in,
	int out
)
{
	// Copy the contents of the file
	char buffer[4096];

	while ( true )
	{
		const ssize_t n = ::read(in, buffer, sizeof( buffer ));

		if ( n < 0 )
		{
			// error
			return false;
		}

		// eof
		if ( n == 0 )
		{
			break;
		}

		// otherwise, copy what we got
		ssize_t x = 0;

		do
		{
			ssize_t m = ::write(out, buffer + x, n - x);

			if ( m == -1 )
			{
				return false;
			}

			x += m;
		}
		while ( x < n );
	}

	// flush to disk
	::fsync(out);
	return true;
}

/* in - an open file descriptor for the input data
 * @todo Linux 3.11 supports O_TMPFILE - an anonymous file that can be "made
 * real". See man open.
 */
bool copy_inner(
	int                in,
	const std::string& dest
)
{
	// Create dest. Possibly in place, but might need a temp file.
	int f = ::open(dest.c_str(), O_CREAT | O_EXCL | O_WRONLY, S_IWUSR | S_IRUSR);

	if ( f != -1 )
	{
		// Copy the file
		if ( !copy_contents(in, f))
		{
			::unlink(dest.c_str());
			::close(f);
			return false;
		}
	}
	else
	{
		// file already exists, so copy to a temp first then overwrite atomically
		std::string       temp_file_name = dest + "cpyXXXXXX";
		const std::size_t n              = temp_file_name.length();
		char*             s              = new char[n + 1];
		temp_file_name.copy(s, n, 0);
		s[n]           = '\0';
		f              = ::mkstemp(s);
		temp_file_name = s;
		delete[] s;

		if ( f != -1 )
		{
			::fchmod(f, S_IWUSR | S_IRUSR);

			// Copy the file
			if ( !copy_contents(in, f) || ( ::rename(temp_file_name.c_str(), dest.c_str()) != 0 ))
			{
				// error occurred. Remove the partial file
				::unlink(temp_file_name.c_str());

				::close(f);
				return false;
			}
		}
		else
		{
			// couldn't even make temporary
			return false;
		}
	}

	// File copy completed successfully. Fix file permissions.
	// change the permissions to match the original file
	struct stat st;

	if ( ::fstat(in, &st) == 0 )
	{
		::fchmod(f, st.st_mode & ( S_IRWXU | S_IRWXG | S_IRWXO ));
	}

	::close(f);

	return true;

}

bool is_file(int in)
{
	struct stat st;

	if ( ::fstat(in, &st) == 0 )
	{
		return S_ISREG(st.st_mode) || S_ISLNK(st.st_mode);
	}

	return false;
}

}

namespace pbl
{
namespace fs
{

/** Try to do copy the file as safely as possible (esp., gracefully handle
 * errors, avoid race conditions).
 *
 * @todo What if dest is a directory?
 * @todo Don't copy over self
 */
bool copy(
	const std::string& source,
	const std::string& dest
)
{
	// Open the input file
	int in = ::open(source.c_str(), O_RDONLY);

	if ( in != -1 )
	{
		bool res = false;

		if ( is_file(in))
		{
			res = copy_inner(in, dest);
		}

		::close(in);
		return res;
	}

	return false;
}

}
}
