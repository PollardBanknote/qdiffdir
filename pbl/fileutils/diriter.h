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
#ifndef DIRITER_H
#define DIRITER_H

#include "fileinfo.h"
#include <dirent.h>

namespace pbl
{
namespace fs
{
/// @todo Cache stat and make it available to the traverse_visitor
/// @todo recursive_directory_iterator
class directory_iterator
{
public:
	// construct an end iterator
	directory_iterator();

	explicit directory_iterator(const std::string& path);

	~directory_iterator();

	bool operator==(const directory_iterator& i) const;

	bool operator!=(const directory_iterator& i) const;

	directory_iterator& operator++();

	const fileinfo_t& operator*() const;

	const fileinfo_t* operator->() const;
private:
	directory_iterator(const directory_iterator&);
	directory_iterator& operator=(const directory_iterator&);

	bool is_end() const;

	void next();

	std::string abspath;
	DIR*        d;
	dirent*     e;
	fileinfo_t  fi;
};

}
}
#endif // DIRITER_H
