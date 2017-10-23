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
#include "filenamematcher.h"

#include "pbl/util/strings.h"

/// @todo file.bak
int FileNameMatcher::compare(
	const std::string& a,
	const std::string& b
) const
{
	if ( a == b )
	{
		return 0;
	}

	if ( gzalt(a, b) )
	{
		return 1;
	}

	if ( a == cppalt(b) )
	{
		return 2;
	}

	if ( a == cgalt(b) )
	{
		return 3;
	}

	return -1;
}

bool FileNameMatcher::gzalt(
	const std::string& s1,
	const std::string& s2
)
{
	const std::size_t n1 = s1.length();
	const std::size_t n2 = s2.length();

	if ( n1 != n2 )
	{
		if ( n1 < n2 )
		{
			if ( s2[n1] == '.' && s2.compare(0, n1, s1) == 0 )
			{
				if ( n2 - n1 == 3 && s2.compare(n1 + 1, std::string::npos, "gz", 2) == 0 )
				{
					return true;
				}
			}
		}
		else if ( s1[n2] == '.' && s1.compare(0, n2, s2) == 0 )
		{
			if ( n1 - n2 == 3 && s1.compare(n2 + 1, std::string::npos, "gz", 2) == 0 )
			{
				return true;
			}
		}
	}

	return false;
}

std::string FileNameMatcher::cppalt(const std::string& s)
{
	if ( pbl::ends_with(s, ".cpp") )
	{
		std::string t(s, 0, s.length() - 2);

		return t;
	}

	if ( pbl::ends_with(s, ".c") )
	{
		return s + "pp";
	}

	return std::string();
}

std::string FileNameMatcher::cgalt(const std::string& s)
{
	if ( pbl::ends_with(s, ".cpp") )
	{
		std::string t(s, 0, s.length() - 2);

		return t + ".gz";
	}

	if ( pbl::ends_with(s, ".c") )
	{
		return s + "pp.gz";
	}

	if ( pbl::ends_with(s, ".c.gz") )
	{
		std::string t(s, 0, s.length() - 3);

		return t + "pp";
	}

	if ( pbl::ends_with(s, ".cpp.gz") )
	{
		std::string t(s, 0, s.length() - 5);

		return t;
	}

	return std::string();
}
