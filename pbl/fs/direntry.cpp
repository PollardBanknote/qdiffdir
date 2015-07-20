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
#include "direntry.h"

namespace pbl
{
namespace fs
{
directory_entry::directory_entry()
{
}

directory_entry::directory_entry(const directory_entry& e)
	: path_(e.path_), mstatus(e.mstatus), sym_status(e.sym_status)
{

}

directory_entry::directory_entry(
    const pbl::fs::path& p,
    file_status          status_,
    file_status          sym_status_
)
	: path_(p), mstatus(status_), sym_status(sym_status_)
{

}

directory_entry& directory_entry::operator=(const directory_entry& e)
{
	path_      = e.path_;
	mstatus    = e.mstatus;
	sym_status = e.sym_status;
	return *this;
}

void directory_entry::assign(
	const pbl::fs::path& p,
	file_status          status_,
	file_status          sym_status_
)
{
	path_      = p;
	mstatus    = status_;
	sym_status = sym_status_;
}

const path& directory_entry::get_path() const
{
	return path_;
}

file_status directory_entry::status() const
{
	return mstatus;
}

file_status directory_entry::symbolic_status() const
{
	return sym_status;
}

}
}
