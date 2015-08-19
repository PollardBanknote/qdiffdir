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
#include "path.h"

namespace pbl
{
namespace fs
{
path::path()
{
}

path::path(const string_type& s_) : s(s_)
{

}

path::path(const char* s_) : s(s_)
{
}

path::path(const path& p) : s(p.s)
{

}

path& path::operator=(const path& p)
{
	s = p.s;
	return *this;
}

path& path::operator=(const string_type& s_)
{
	s = s_;
	return *this;
}

void path::clear()
{
	s.clear();
}

void path::swap(path& p)
{
	s.swap(p.s);
}

std::string path::string() const
{
	return s;
}

const char* path::c_str() const
{
	return s.c_str();
}

bool path::empty() const
{
	return s.empty();
}

path path::filename() const
{
	const std::size_t i = s.find_last_of(preferred_separator);

	if ( i == std::string::npos )
	{
		return *this;
	}

	if ( i + 1 < s.length())
	{
		return path(s.substr(i + 1));
	}
	else
	{
		return "/";
	}
}

path::operator string_type() const
{
	return s;
}

path& path::append(const path& p)
{
	bool sep = true;

	if ( !s.empty() && s[s.length() - 1] == preferred_separator )
	{
		sep = false;
	}
	else if ( s.empty())
	{
		sep = false;
	}
	else if ( p.empty() || p.s[0] == preferred_separator )
	{
		sep = false;
	}

	if ( sep )
	{
		s += preferred_separator;
	}

	s.append(p.native());
	return *this;
}

const std::string& path::native() const
{
	return s;
}

path operator/(
	const path& lhs,
	const path& rhs
)
{
	path res = lhs;

	res.append(rhs);
	return res;
}

std::ostream& operator<<(
	std::ostream& os,
	const path&   p
)
{
	return os << p.string();
}

}
}
