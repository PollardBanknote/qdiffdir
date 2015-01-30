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
#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>

namespace pbl
{
namespace file
{
/** @brief Get an absolute path of a file
 * @param filepath The path of an (existing) file
 * @returns An absolute path that corresponds to the same file as filepath, or
 *   an empty string if there is an error.
 */
std::string absolute_path(const std::string& filepath);

/** @brief Copy the file at source to dest
 * @param source A file to copy
 * @param dest A file (including name) of the file to create
 * @returns true iff dest exists and is a copy of source
 *
 * @note If source does not exist, dest is not created. Will overwrite an
 * existing file "dest". Dest has same permissions as source, if possible.
 */
bool copy(const std::string& source, const std::string& dest);

/** @brief Compare two files to see if they are exactly the same
 * @returns 0 if not the same, 1 if the same, -1 if an error occurred
 */
int compare(const std::string& f, const std::string& g);

/** @brief Return the name of the file system component indicated by path
 * @param path A file path
 * @returns The name of the directory or file that the path points to, or the
 * empty string if there's an error
 *
 * The exact behavior of this function is platform dependent, because paths are.
 * However, whatever the platform, it should return the name of the file or
 * directory pathed. Ex., on POSIX platforms it should return "readme.txt" for
 * "/home/user/documents/readme.txt"; similarly it should return "readme.txt"
 * for "C:\Windows\readme.txt" on Windows platforms.
 *
 * This function does not access the file system. It merely parses the string.
 * In particular, it does not check if the path is valid and/or accessible.
 *
 * If the last component cannot be determined for whatever reason, the string is
 * considered malformed and the empty string is returned. Some examples of
 * a "malformed" string are "", ".", "..", "usr/..". (However, something like
 * "usr/lib/.." should return "usr".)
 */
std::string basename(const std::string& path);

}
}

#endif // FILEUTILS_H
