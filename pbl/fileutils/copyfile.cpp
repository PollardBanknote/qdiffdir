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
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>

#ifdef _POSIX_C_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "pbl/fileutils/fileutils.h"

namespace
{
/// @todo This is a bad way to get file existence
bool file_exists(const std::string& filename)
{
    return !pbl::file::absolute_path(filename).empty();
}

}

namespace pbl
{
namespace file
{

/** Try to do copy the file as safely as possible.
 *
 * @todo Should group permissions be copied? Both user permissions and other
 * permissions should probably be the same. But group might be a different
 * group.
 * @todo Relying so heavily on C, seems strange to use ifstream
 * @todo What if dest is a directory? What if source is a directory?
 */
bool copy(
	const std::string& source,
	const std::string& dest
)
{
	// Open the input file
	std::ifstream in(source.c_str(), std::ios_base::in | std::ios_base::binary);

	if ( !in.is_open())
	{
		return false;
	}

	// change the permissions to match the original file
	mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;

	struct stat st;

	if ( ::stat(source.c_str(), &st) == 0 )
	{
		mode &= st.st_mode;
	}

	// Create dest. Possibly in place, but might need a temp file.
	std::string temp_file_name;

	int f = -1;

	if ( !file_exists(dest))
	{
		// file doesn't exist, create it directly
		/// @todo Probably some race condition here. Might prefer open with
		/// O_CREAT and falling back to the "file exists" method
		f = ::creat(dest.c_str(), S_IWUSR | S_IRUSR);
	}
	else
	{
		// file exists, so use a temp to receive the copy before overwriting
		temp_file_name = dest + "cpyXXXXXX";
		const std::size_t n = temp_file_name.length();
		char*             s = new char[n + 1];
		temp_file_name.copy(s, n, 0);
		s[n] = '\0';
		f    = ::mkstemp(s);

		if ( f != -1 )
		{
			::fchmod(f, S_IWUSR | S_IRUSR);
		}

		temp_file_name = s;
		delete[] s;
	}

	if ( f == -1 )
	{
		return false;
	}

	// Copy the contents of the file
	char buffer[4096];

	while ( in )
	{
		in.read(buffer, sizeof( buffer ));

		if ( !in.bad())
		{
			::write(f, buffer, in.gcount());
		}
		else
		{
			// error occurred. Remove the partial file
			if ( !temp_file_name.empty())
			{
				::unlink(temp_file_name.c_str());
			}
			else
			{
				::unlink(dest.c_str());
			}

			::close(f);
			return false;
		}
	}

	// Clean up the temp
	if ( !temp_file_name.empty())
	{
		// commit the new file
		int res = ::rename(temp_file_name.c_str(), dest.c_str());

		if ( res != 0 )
		{
			// Failed. Remove the temp file
			::remove(temp_file_name.c_str());

			return false;
		}
	}

	// File copy completed successfully. Fix file permissions.
	::fchmod(f, mode);
	::close(f);

	return true;
}
}
}
