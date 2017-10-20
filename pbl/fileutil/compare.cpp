/* Copyright (c) 2015, Pollard Banknote Limited
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
#include "compare.h"

#include <cstring>

#if !defined( _WIN32 ) && ( defined( __unix__ ) || defined( __unix ) || ( defined( __APPLE__ ) && defined( __MACH__ )  ) )
#include <unistd.h>
#if defined( _POSIX_VERSION )
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#endif

namespace pbl
{
namespace fs
{
compare_result compare(
	const std::string& first,
	const std::string& second,
	long long          sizelimit
)
{
	compare_result res(compare_error);

	if (std::FILE* fd1 = std::fopen(first.c_str(), "rb"))
	{
		if (std::FILE* fd2 = std::fopen(second.c_str(), "rb"))
		{
			res = compare(fd1, fd2, sizelimit);

			std::fclose(fd2);
		}
		std::fclose(fd1);
	}

	return res;
}

compare_result compare(
    std::FILE*       file1,
    std::FILE*       file2,
	long long sizelimit
)
{
	{
		/* Check if the files are obviously the same or different. Ex., because
		 * of file size or hardlinks.
		 */
		int fd1 = ::fileno(file1);
		int fd2 = ::fileno(file2);

		if (fd1 != -1 && fd2 != -1)
		{
			struct stat s1;
			struct stat s2;

			const bool res1 = ::fstat(fd1, &s1) == 0;
			const bool res2 = ::fstat(fd2, &s2) == 0;

			if ( res1 && res2 )
			{
				// files of different size are obviously different
				if ( s1.st_size != s2.st_size )
				{
					return compare_result(compare_notequal);
				}

				// files with the same dev/inode are obviously the same and don't need to
				// be compared
				if ( s1.st_ino == s2.st_ino && s1.st_dev == s2.st_dev )
				{
					return compare_result(compare_equal);
				}
			}

			// Don't check files that are larger than the size limit
			if ( sizelimit != 0 && ( ( res1 && s1.st_size > sizelimit ) || ( res2 && s2.st_size > sizelimit ) ) )
			{
				return compare_result(compare_error);
			}
		}
	}

	// Start reading buffers
	char buf1[4096];
	char buf2[4096];

	std::size_t size1 = 0;
	std::size_t size2 = 0;

	bool eof1 = false;
	bool eof2 = false;

	while ( true )
	{
		// read from each file
		if ( !eof1 && size1 < sizeof( buf1 ) )
		{
			const std::size_t m1 = sizeof( buf1 ) - size1;
			const std::size_t n1 = std::fread(buf1 + size1, 1, m1, file1);

			if (n1 < m1)
			{
				// eof or error
				if (std::ferror(file1))
				{
					return compare_result(compare_error);
				}

				// must be end of file, then
				eof1 = true;
			}

			size1 += n1;
		}

		if ( !eof2 && size2 < sizeof( buf2 ) )
		{
			const std::size_t m2 = sizeof( buf2 ) - size2;
			const std::size_t n2 = std::fread(buf2 + size2, 1, m2, file2);

			if (n2 < m2)
			{
				// eof or error
				if (std::ferror(file2))
				{
					return compare_result(compare_error);
				}

				// must be end of file, then
				eof2 = true;
			}

			size2 += n2;
		}

		// Compare files based on what we have
		const std::size_t m = std::min(size1, size2);

		// files are different
		if ( std::memcmp(buf1, buf2, m) != 0 )
		{
			return compare_result(compare_notequal);
		}
		else
		{
			// files are the same
			if ( eof1 && eof2 )
			{
				return compare_result(compare_equal);
			}

			// files are the same so far... save the data
			if ( size1 > m )
			{
				std::memmove(buf1, buf1 + m, size1 - m);
			}

			size1 -= m;

			if ( size2 > m )
			{
				std::memmove(buf2, buf2 + m, size2 - m);
			}

			size2 -= m;

			// files have different size
			if ( ( eof1 && size2 != 0 ) || ( eof2 && size1 != 0 ) )
			{
				return compare_result(compare_notequal);
			}
		}
	}

	return compare_result(compare_error);
}

}
}
