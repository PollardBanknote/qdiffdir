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
#ifndef PBL_FILEUTIL_COMPARE_H
#define PBL_FILEUTIL_COMPARE_H

#include <string>
#include <cstdio>
#include "../util/return_code.h"

namespace pbl
{
namespace fs
{
enum compare_result_enum
{
	compare_equal,
	compare_notequal_sizes,
	compare_notequal_content,
	compare_error_open,
	compare_error_null,
	compare_error_too_big,
	compare_error_read
};

typedef return_code< compare_result_enum > compare_result;

compare_result compare(const std::string&, const std::string&, long long);
compare_result compare(std::FILE*, std::FILE*, long long);
}
}

#endif // PBL_FILEUTIL_COMPARE_H
