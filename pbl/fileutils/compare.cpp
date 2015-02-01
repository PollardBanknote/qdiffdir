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
#include <cstdio>
#include <cstring>

#ifdef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace
{
/** Do two files have different sizes
 *
 * Only returns true if file sizes could be correctly determined. If
 * a file is inaccessible, for example, function will return false
 *
 * See FIO19-C from cert for problems with getting file size
 */
bool different_sizes(const std::string& f, const std::string& g)
{
#ifdef _POSIX_C_SOURCE
	struct stat buf1;
	struct stat buf2;

	if (stat(f.c_str(), &buf1) == 0 && stat(g.c_str(), &buf2) == 0)
	{
		return buf1.st_size != buf2.st_size;
	}
	return false;
#else
#warning "No implementation of different_sizes for this platform"
	return false;
#endif
}
}

namespace pbl
{
namespace fs
{
int compare(const std::string& f, const std::string& g)
{
	if (different_sizes(f, g))
		return 0;

	FILE* f1 = fopen(f.c_str(), "rb");
	if (!f1)
	{
		return -1;
	}

	FILE* f2 = fopen(g.c_str(), "rb");
	if (!f2)
	{
		fclose(f1);
		return -1;
	}

	char buf1[4096];
	char buf2[4096];

	int result = 1;

	while (true)
	{
		const size_t n1 = fread(buf1, 1, sizeof(buf1), f1);
		const bool eof1 = (n1 != sizeof(buf1));
		if (eof1 && ferror(f1))
		{
			result = -1;
			break;
		}
		const size_t n2 = fread(buf2, 1, sizeof(buf2), f2);
		const bool eof2 = (n2 != sizeof(buf2));
		if (eof2 && ferror(f2))
		{
			result = -1;
			break;
		}

		if (n1 != n2 || memcmp(buf1, buf2, n1) != 0)
		{
			result = 0;
			break;
		}

		if (eof1 && eof2)
			break;
	}

	fclose(f2);
	fclose(f1);
	return result;
}

}
}
