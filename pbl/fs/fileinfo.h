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
#ifndef FILEINFO_H
#define FILEINFO_H
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
struct dirent;

namespace pbl
{
namespace fs
{

/** A class for accessing information about a file
 *
 * Users should be aware that this represents a file at a certain point in
 * time and may not be current
 *
 * @bug Paths passed in should be absolute, or appended to cw
 */
class fileinfo_t
{
	/** Tracks which pieces of information are available
	 */
	enum flags_t
	{
		MODE = 1, ///< file type
		ALL  = 1  ///< Everything is available
	};
public:
	fileinfo_t();

	/** Get information about a file
	 *
	 * @param path Path to a file
	 */
	explicit fileinfo_t(const std::string& path);

	/** Get information about a file
	 *
	 * @param dir Name of the file
	 * @param e cannot be NULL
	 *
	 * This constructor is used by the directory_iterator to partially
	 * initialize the fileinfo_t. This, hopefully, can save further disk
	 * access.
	 */
	fileinfo_t(
	    const std::string& dir,
	    const dirent*      e
	);

	/** Copy constructor
	 */
	fileinfo_t(const fileinfo_t& i);

	/** Copy assignment
	 */
	fileinfo_t& operator=(const fileinfo_t& i);

	/** The absolute path of the file
	 */
	std::string absolute_path() const;

	/** Name of the file
	 */
	std::string name() const;

	/** Check if the file is a directory
	 */
	bool is_directory() const;
private:
	/// Full path to file
	std::string path_;

	/// Name of the file
	std::string name_;

	/// Which values are set
	mutable int flags;

	/// Platform information for the file
	mutable struct stat st;
};

}
}

#endif // FILEINFO_H
