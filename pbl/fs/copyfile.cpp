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
#include "file.h"

namespace pbl
{
namespace fs
{

/** Try to do copy the file as safely as possible (esp., gracefully handle
 * errors, avoid race conditions).
 *
 * @todo What if source is a device? /dev/random, /dev/null
 * @todo What if dest is a directory?
 * @todo Don't copy over self
 */
bool copy_file(
	const std::string& source,
	const std::string& dest
)
{
	// Open the input file
	file in(source, file::readonly);

	if ( in.is_open() && in.is_file())
	{
		file out(dest, file::create | file::writeonly, perms::owner_read | perms::owner_write);

		if ( out.is_open())
		{
			// Copy the file to the newly created file
			if ( out.copy(in))
			{
				out.chmod(in);
				return true;
			}
			else
			{
				// remove the incomplete copy
				out.remove();
				return false;
			}
		}
		else
		{
			file tmp; /// @todo pbl::fs::tempfile

			/// @bug if(errno == EEXIST)
			if ( tmp.mkstemp(dest) && tmp.copy(in) && tmp.realize(dest))
			{
				tmp.chmod(in);
				return true;
			}
		}

		return false;
	}

	return false;
}

}
}
