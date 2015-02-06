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
#include "diriter.h"
#include <cstring>
#include "fileutils.h"
namespace pbl
{
namespace fs
{

directory_iterator::directory_iterator() : d(0), e(0)
{
}

directory_iterator::directory_iterator(const std::string& path) : d(0), e(0)
{
	d = opendir(path.c_str());

	if ( d )
	{
		// get the absolute path of the directory
		if ( !path.empty() && path[0] == '/' )
		{
			abspath = path;
		}
		else
		{
			abspath = absolute_path(path);
		}

		next();
	}
}

directory_iterator::~directory_iterator()
{
	if ( d )
	{
		closedir(d);
	}
}

bool directory_iterator::operator==(const directory_iterator& i) const
{
	// only equal if both are end iterators
	return is_end() && i.is_end();
}

bool directory_iterator::operator!=(const directory_iterator& i) const
{
	return !( is_end() && i.is_end());
}

directory_iterator& directory_iterator::operator++()
{
	next();
	return *this;
}

const fileinfo_t& directory_iterator::operator*() const
{
	return fi;
}

const fileinfo_t* directory_iterator::operator->() const
{
	return &fi;
}

bool directory_iterator::is_end() const
{
	return !d || !e;
}

void directory_iterator::next()
{
	if ( d )
	{
		do
		{
			e = readdir(d);
		}
		while ( e && ( strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0 ));

		if ( e )
		{
			fi = fileinfo_t(abspath, e);
		}
		else
		{
			fi = fileinfo_t();
		}
	}
}

}
}
