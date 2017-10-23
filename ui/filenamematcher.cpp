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
FileNameMatcher::match_result FileNameMatcher::compare(
	const std::string& a,
	const std::string& b
) const
{
	if ( a == b )
	{
		match_result t = { 0, "", "" };
		return t;
	}

	// Archives should by un-archived
	if ( b == a + ".gz" )
	{
		match_result t = { 1, "", "gunzip -c" };
		return t;
	}

	if (a == b + ".gz")
	{
		match_result t = { 1, "gunzip -c", "" };
		return t;
	}

	// C and C++ files can be compared directly
	if ( (pbl::ends_with(b, ".cpp") && (a + "pp" == b))
	     || (pbl::ends_with(a, ".cpp") && (b + "pp" == a)))
	{
		match_result t = { 2, "", "" };
		return t;
	}

	if (pbl::ends_with(b, ".cpp") && pbl::ends_with(a, ".c.gz") && a.compare(0, a.length() - 5, b, 0, b.length() - 4) == 0)
	{
		match_result t = { 3, "gunzip -c", "" };
		return t;
	}

	if (pbl::ends_with(a, ".cpp") && pbl::ends_with(b, ".c.gz") && b.compare(0, b.length() - 5, a, 0, a.length() - 4) == 0)
	{
		match_result t = { 3, "", "gunzip -c" };
		return t;
	}

	if (pbl::ends_with(a, ".cpp.gz") && a == b + "pp.gz")
	{
		match_result t = { 3, "gunzip -c", "" };
		return t;
	}

	if (pbl::ends_with(b, ".cpp.gz") && b == a + "pp.gz")
	{
		match_result t = { 3, "", "gunzip -c" };
		return t;
	}

	match_result fail = { -1, "", "" };
	return fail;
}
