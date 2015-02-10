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
#include "fileinfo.h"

#include <dirent.h>
#include "fileutils.h"

namespace pbl
{
namespace fs
{

fileinfo_t::fileinfo_t() : flags(0)
{
}

fileinfo_t::fileinfo_t(const std::string& path)
	: path_(path), name_(basename(path)), flags(0)
{
}

fileinfo_t::fileinfo_t(
    const std::string& dir,
    const dirent*      e
)
	: flags(0)
{
	path_ = dir + "/" + e->d_name;
	name_ = e->d_name;

	// convert the dirent information to stat
	if ( e->d_type != DT_UNKNOWN && e->d_type != DT_LNK )
	{
		st.st_mode = DTTOIF(e->d_type);
		flags      = MODE;
	}
}

fileinfo_t::fileinfo_t(const fileinfo_t& i)
	: path_(i.path_), name_(i.name_), flags(i.flags), st(i.st)
{
}

fileinfo_t& fileinfo_t::operator=(const fileinfo_t& i)
{
	path_ = i.path_;
	name_ = i.name_;
	flags = i.flags;
	st    = i.st;
	return *this;
}

std::string fileinfo_t::absolute_path() const
{
	return pbl::fs::absolute_path(path_);
}

std::string fileinfo_t::name() const
{
	return name_;
}

bool fileinfo_t::is_directory() const
{
	// do we already know?
	if (( flags & MODE ) == 0 )
	{
		stat(path_.c_str(), &st);
		flags = flags | ALL;
	}

	return S_ISDIR(st.st_mode);
}

}
}
