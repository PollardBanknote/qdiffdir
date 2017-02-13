/* Copyright (c) 2017, Pollard Banknote Limited
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
#include "which.h"

#include <cstdlib>
#include <cerrno>

#include <unistd.h>

namespace
{
/// @todo Move to system/user.cpp ?
std::string getpath()
{
	std::string s;

	if ( const char* path = std::getenv("PATH"))
	{
		s = path;
	}

	return s;
}

// file exists and can be executed
bool is_executable(const std::string& path)
{
	return ::access(path.c_str(), X_OK) == 0;
}

std::string myrealpath(const std::string& path)
{
	std::string s;

	// relative path
	#if (( defined( _POSIX_VERSION ) && _POSIX_VERSION >= 200809l ) || defined( __GLIBC__ ))

	if ( char* res = ::realpath(path.c_str(), NULL))
	{
		s = res;
		::free(res);
	}

	#else
	#error "Need to resolve relative path"
	#endif

	return s;
}

}

namespace pbl
{
std::string which(const std::string& filename)
{
	// Explicitly pathed executables are not searched
	if ( filename.find('/') != std::string::npos )
	{
		if ( filename[0] == '/' )
		{
			// absolute path
			if ( is_executable(filename))
			{
				return filename;
			}
		}
		else
		{
			std::string abs = myrealpath(filename);

			if ( is_executable(abs))
			{
				return abs;
			}
		}
	}
	else if ( !filename.empty())
	{
		/// @todo Provide an iterator over the parts of the string
		const std::string path = getpath();
		const std::size_t n    = path.length();

		for ( std::size_t i = 0; i < n;)
		{
			const std::size_t j = path.find(':', i);

			const std::size_t len = ( j != std::string::npos ? j - i : n - i );

			std::string prefix;

			if ( len != 0 )
			{
				prefix.assign(path, i, len);
			}
			else
			{
				prefix = ".";
			}

			const std::string full = prefix + "/" + filename;

			if ( is_executable(full))
			{
				return full;
			}

			i = i + len + 1;
		}

		// PATH may end with a colon, meaning cwd
		if ( n != 0 && path[n - 1] == ':' )
		{
			const std::string full = "./" + filename;

			if ( is_executable(full))
			{
				return full;
			}
		}
	}

	return std::string();
}

}
