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

directory_iterator::directory_iterator(const path& path_) : d(0), e(0)
{
	d = opendir(path_.c_str());

	if ( d )
	{
		dirpath = path_;

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

const directory_entry& directory_iterator::operator*() const
{
	return fi;
}

const directory_entry* directory_iterator::operator->() const
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
			file_status s;
			file_status ss;

			switch ( e->d_type )
			{
			case DT_UNKNOWN:
				break;
			case DT_FIFO:
				s.type(file_type::fifo);
				break;
			case DT_CHR:
				s.type(file_type::character);
				break;
			case DT_DIR:
				s.type(file_type::directory);
				break;
			case DT_BLK:
				s.type(file_type::block);
				break;
			case DT_REG:
				s.type(file_type::regular);
				break;
			case DT_LNK:
				/// @todo fill in ss
				s.type(file_type::symlink);
				break;
			case DT_SOCK:
				s.type(file_type::socket);
				break;
			default:
				break;
			} // switch

			/// @todo Relative path?
			fi = directory_entry(dirpath / path(e->d_name), s, ss);
		}
		else
		{
			fi = directory_entry();
		}
	}
}

}
}
