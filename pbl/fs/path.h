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
#ifndef PATH_H
#define PATH_H
#include <string>

namespace pbl
{
namespace fs
{
/** A file path
 *
 * A partial implementation of std::experimental::filesystem::path
 */
class path
{
public:
	typedef std::string string_type;
	typedef char value_type;

	static const value_type preferred_separator = '/';

	/// An empty path
	path();

	/// Construct a path form the given string
	explicit path(const string_type&);

	/// Copy constructor
	path(const path&);

	/// Copy assignment
	path& operator=(const path&);

	path& operator=(const string_type&);

	/// Clear the path
	void clear();

	/// Swap the two paths
	void swap(path&);

	std::string string() const;
	const char* c_str() const;
	path& append(const path&);

	/// Is the path empty (i.e., "")
	bool empty() const;
	const std::string& native() const;

	path filename() const;
private:
	std::string s;
};

path operator/(const path& lhs, const path& rhs);
}
}

#endif // PATH_H
