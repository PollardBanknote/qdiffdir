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

#include <dirent.h>

#include "fileinfo.h"

namespace pbl
{
namespace fs
{
/** An iterator for traversing directory contents
 *
 * This class can be used for iterating over the contents of a directory. The
 * parameterized constructor will make a "begin" iterator, and the default
 * constructor will make an "end" iterator. One can dereference an iterator
 * to get a fileinfo_t describing the file/subdirectory.
 *
 * This class may not be very useful to a user, as it has several limitations
 * due to the asynchronous nature of the file system (and platform details).
 *
 * @note The "." and ".." 'files' are ignored by this iterator, since they are
 * not 'in' the directory.
 *
 * @note These objects are not copyable
 *
 * @note Only end iterators are ever considered equal. In particular, two
 * directory_iterator-s constructed with the same argument may not point to the
 * same file.
 *
 * As a point of interest, this is not meant as a re-implementation of
 * boost::directory_iterator. It is largely meant for internal use by other fs
 * classes and functions. The similarities to boost are actually coincidental.
 *
 * @todo recursive_directory_iterator
 */
class directory_iterator
{
public:
	/** Construct an end iterator
	 */
	directory_iterator();

	/** Construct an iterator for a directory
	 *
	 * After construction, the iterator will point to the first file in the
	 * directory.
	 */
	explicit directory_iterator(const std::string& path);

	/** Destructor
	 */
	~directory_iterator();

	/** Test if two iterators are equal
	 *
	 * Only two end iterators are equal
	 */
	bool operator==(const directory_iterator& i) const;

	/** Test if two iterators are not equal
	 *
	 * Only two end iterators are equal
	 */
	bool operator!=(const directory_iterator& i) const;

	/** Move to the next file/subdirectory
	 */
	directory_iterator& operator++();

	/** Get information for the file
	 */
	const fileinfo_t& operator*() const;

	/** Get information for the file
	 */
	const fileinfo_t* operator->() const;
private:
	// non-copyable
	directory_iterator(const directory_iterator&);
	directory_iterator& operator=(const directory_iterator&);

	/** Test if this is an end iterator
	 */
	bool is_end() const;

	/** Move to the next file/subdirectory
	 */
	void next();

	/// Path to the directory
	std::string abspath;

	/// Platform information
	DIR* d;

	/// Platform information
	dirent* e;

	/// Information about the file/subdirectory
	fileinfo_t fi;
};

}
}
#endif // DIRITER_H
