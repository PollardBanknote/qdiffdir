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
#ifndef FILE_H
#define FILE_H

#include <string>

#if !defined( _WIN32 ) && ( defined( __unix__ ) || defined( __unix ) || ( defined( __APPLE__ ) && defined( __MACH__ )))
#include <unistd.h>
#if defined( _POSIX_VERSION )
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#endif

#include "cpp/filesystem.h"

namespace pbl
{
namespace fs
{

class file
{
public:
	enum flags_t
	{
		create    = O_CREAT | O_EXCL,
		readonly  = O_RDONLY,
		writeonly = O_WRONLY,
		readwrite = O_RDWR
	};

	typedef ssize_t size_type;

	file();
	file(const std::string& name, int flags);
	file(const std::string& name, int flags, ::pbl::fs::perms);
	~file();

	bool is_open() const;

	/// @bug if symlink, then is the link end a file
	bool is_file() const;

	size_type read(char* buffer, std::size_t n);

	size_type write(const char* buffer, std::size_t n);

	::pbl::fs::perms permissions() const;

	void flush();

	void remove();

	void chmod(::pbl::fs::perms m);

	void chmod(const file&);

	/// @todo Decide on precise behavior and how "name" is interpreted. Ex.,
	/// temp file should be made in same directory as name
	bool mkstemp(const std::string& name);

	bool rename(const std::string& dest);

	/// Make a temporary file "real" (by renaming it)
	bool realize(const std::string& dest);

	// append contents of in. Both file descriptors must be valid
	bool copy(file& in);

	size_type size() const;

	// 0 - files are not the same, 1 files are the same
	// -1 some kind of error happened
	int compare(const file&);
private:
	class seek_guard;

	file(const file&);
	file& operator=(const file&);
	bool get_stat() const;

	std::string filename;

	int                  fd;
	bool                 is_temp;
	mutable struct stat* filestat;
};
}
}

#endif // FILE_H
