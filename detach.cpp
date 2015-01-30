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
#include "detach.h"

#include <cstdlib>
#include <cstdio>

#ifdef _POSIX_C_SOURCE
#include <unistd.h>
#endif

namespace pbl
{
namespace process
{

/* We use _Exit to avoid calling destructors, etc.
 */
void detach()
{
	#if defined( _POSIX_VERSION )

	// Detach process
	pid_t res = fork();

	if ( res != -1 )
	{
		if ( res != 0 )
		{
			// in parent - shut it down
			_Exit(EXIT_SUCCESS);
		}

		// in first child. Try to fork again to avoid zombies and the
		// possibility of regaining a controlling terminal
		if ( setsid() != -1 )
		{
			// try to create grandchild
			res = fork();

			if ( res != -1 )
			{
				if ( res != 0 )
				{
					_Exit(EXIT_SUCCESS);
				}
			}
		}

		freopen("/dev/null","r",stdin);
		freopen("/dev/null", "a", stdout);
		freopen("/dev/null", "a", stderr);
	}

	#endif // if defined( _POSIX_VERSION )
}
}
}
