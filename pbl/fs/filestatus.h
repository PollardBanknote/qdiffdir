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
#ifndef FILESTATUS_H
#define FILESTATUS_H

#include "filetype.h"
#include "perms.h"

namespace pbl
{
namespace fs
{
/** Information about a file's type and permissions
 *
 * Due to the nature of the file system, the information may not be exactly
 * current.
 */
class file_status
{
public:
	file_status(const file_status&);
	explicit file_status(file_type = file_type::none, perms = perms::unknown);
	file_status& operator=(const file_status&);
	file_type type() const;
	void type(file_type);
	perms permissions() const;
	void permissions(perms);
private:
	file_type t;
	perms     p;
};
}
}

#endif // FILESTATUS_H
