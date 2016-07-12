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
#include "filestatus.h"

#include <iostream>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "path.h"

namespace
{
::pbl::fs::file_status from_mode_t(mode_t m)
{
	::pbl::fs::perms p = static_cast< ::pbl::fs::perms >( m & 0xFFF );

	::pbl::fs::file_type t = file_type::unknown;

	if ( S_ISREG(m))
	{
		t = file_type::regular;
	}
	else if ( S_ISDIR(m))
	{
		t = file_type::directory;
	}
	else if ( S_ISCHR(m))
	{
		t = file_type::character;
	}
	else if ( S_ISBLK(m))
	{
		t = file_type::block;
	}
	else if ( S_ISFIFO(m))
	{
		t = file_type::fifo;
	}
	else if ( S_ISLNK(m))
	{
		t = file_type::symlink;
	}
	else if ( S_ISSOCK(m))
	{
		t = file_type::socket;
	}

	return ::pbl::fs::file_status(t, p);

}

}

namespace pbl
{
namespace fs
{
file_status::file_status(const file_status& s)
	: t(s.t), p(s.p)
{

}

file_status::file_status(file_type t_, perms p_)
	: t(t_), p(p_)
{

}

file_status& file_status::operator=(const file_status& s)
{
	t = s.t;
	p = s.p;
	return *this;
}

file_type file_status::type() const
{
	return t;
}

void file_status::type(file_type t_)
{
	t = t_;
}

perms file_status::permissions() const
{
	return p;
}

void file_status::permissions(perms p_)
{
	p = p_;
}

std::ostream& operator<<(
	std::ostream&      os,
	const file_status& fs
)
{
	os << fs.type() << "; " << fs.permissions();
	return os;
}

file_status status(const path& path_)
{
	if ( !path_.empty())
	{
		struct stat st;

		if ( ::stat(path_.c_str(), &st) == 0 )
		{
			return from_mode_t(st.st_mode);
		}
	}

	return file_status();
}

file_status symlink_status(const path& path_)
{
	if ( !path_.empty())
	{
		struct stat st;

		if ( ::lstat(path_.c_str(), &st) == 0 )
		{
			return from_mode_t(st.st_mode);
		}
	}

	return file_status();
}

bool is_symlink(file_status s)
{
	return s.type() == file_type::symlink;
}

}
}
