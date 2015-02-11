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
#include <stdexcept>

#include "fileutils.h"

namespace
{
std::size_t after_last(const std::string& s, char c, std::size_t j)
{
	std::size_t i = s.find_last_of(c, j);
	return i == std::string::npos ? 0 : i + 1;
}

// (first character, length) of last path component in s
// or (npos, 0) if error
std::pair< std::size_t, std::size_t > locate_last_path_component(const std::string& s)
{
	if (s.empty())
		return std::pair< std::size_t, std::size_t >(std::string::npos, 0);

	// j points to the last character in a component
	std::size_t j     = s.find_last_not_of('/');
	unsigned    depth = 0;

	while ( j != std::string::npos )
	{
		// i points to first character of a component
		// i <= j, because j never points to a '/'
		const std::size_t i = after_last(s, '/', j);

		if ( j - i + 1 == 1 && s[i] == '.' )
		{
			// component is ".", basically ignore this component
		}
		else if ( j - i + 1 == 2 && s[i] == '.' && s[i + 1] == '.' )
		{
			// component is "..", ignore the next component
			++depth;
		}
		else
		{
			// found a "normal" path component
			if ( depth == 0 )
			{
				return std::pair< std::size_t, std::size_t >(i, j - i + 1);
			}

			// ..but we're ignoring it
			--depth;
		}

		if ( i == 0 )
		{
			// error, path is malformed
			return std::pair< std::size_t, std::size_t >(std::string::npos, 0);
		}

		j = s.find_last_not_of('/', i - 1);
	}

	// all slashes
	return std::pair< std::size_t, std::size_t >(0, 1);
}

std::string basename_posix(const std::string& s)
{
	const std::pair< std::size_t, std::size_t > range = locate_last_path_component(s);

	if (range.first == std::string::npos)
		return std::string();
	return s.substr(range.first, range.second);
}

std::string dirname_posix(const std::string& s)
{
	const std::pair< std::size_t, std::size_t > range = locate_last_path_component(s);

	// path was malformed
	if (range.first == std::string::npos)
		return std::string();

	if (range.first == 0)
	{
		// could be '/', or could be a unpathed filename
		if (s[0] == '/')
			return "/";
		return ".";
	}
	else
	{
		// leading directory
		return s.substr(0, range.first);
	}
}

}

namespace pbl
{
namespace fs
{
// Calls the basename_xxx appropriate for this platform
std::string basename(const std::string& s)
{
	#ifdef _POSIX_C_SOURCE
	return basename_posix(s);

	#else
	#error "No implementation of basename is available for this platform"
	#endif
}

std::string dirname(const std::string& s)
{
	#ifdef _POSIX_C_SOURCE
	return cleanpath(dirname_posix(s));
	#else
	#error "No implementation of dirname is available for this platform"
	#endif
}

#if 0

void test_basename()
{
	// (input, basename)
	const char* const paths[][2] =
	{
		{ "/", "/" },
		{ "///", "/" },
		{ "/./", "/" },
		{ "/../", "/" },
		{ "/one/../", "/" },
		{ "lib", "lib" },
		{ "lib/", "lib" },
		{ "/lib", "lib" },
		{ "/lib/", "lib" },
		{ "usr/lib", "lib" },
		{ "usr/lib", "lib" },
		{ "/usr/lib", "lib" },
		{ "/usr/lib/", "lib" },
		{ "lib///", "lib" },
		{ "///lib", "lib" },
		{ "///lib///", "lib" },
		{ "usr///lib", "lib" },
		{ "///usr///lib", "lib" },
		{ "///usr///lib///", "lib" },
		{ "///usr///lib///", "lib" },
		{ "///usr///lib///", "lib" },
		{ "/lib/.", "lib" },
		{ "lib/.", "lib" },
		{ "lib/./", "lib" },
		{ "/lib/one/../", "lib" },
		{ "/lib/one/..", "lib" },
		{ "lib/one/..", "lib" },
		{ "lib/one/../", "lib" },
		{ "lib/one/two/../../", "lib" },
		{ "", "" },
		{ "./", "" },
		{ "usr/..", "" },
	};

	const std::size_t n = sizeof( paths ) / sizeof( paths[0] );

	for ( std::size_t i = 0; i < n; ++i )
	{
		if ( basename_posix(paths[i][0]) != paths[i][1] )
		{
			throw std::runtime_error("Bad implementation of basename_posix");
		}
	}
}

#endif  // if 0

}
}
