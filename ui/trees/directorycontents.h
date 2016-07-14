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
#ifndef DIRECTORYCONTENTS_H
#define DIRECTORYCONTENTS_H

#include <vector>
#include <string>
#include "cpp/filesystem.h"

/** Used to observe the files/subdirectories of a directory
 */
class DirectoryContents
{
public:
	struct update_t
	{
		std::vector< std::string > added;
		std::vector< std::string > removed;
		std::vector< std::string > changed;
	};

	DirectoryContents();

	std::string absolutePath() const;

	std::string absoluteFilePath(const std::string& s) const;

	std::string relativeFilePath(const std::string& s) const;

	std::string name() const;

	bool cd(const std::string& path);

	std::vector< std::string > setDepth(int d);

	std::vector< std::string > getRelativeFileNames() const;

	std::vector< std::string > getDirectories() const;

	std::vector< std::string > getAbsoluteFileNames() const;

	update_t update(const std::string& d);
private:
	void refresh();

	// an absolute path to the top directory
	std::string dir;

	int maxdepth;

	// relative paths of each file
	std::vector< std::string > files;

	// absolute paths of directories that we should watch
	std::vector< std::string > subdirs;
};


#endif // DIRECTORYCONTENTS_H
