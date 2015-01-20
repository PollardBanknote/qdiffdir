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

#include <QFileInfoList>
#include <QDir>
#include <QString>

/** @brief Get an absolute path of a file
 * @param filepath The path of an (existing) file
 * @returns An absolute path that corresponds to the same file as filepath, or
 *   an empty string if there is an error.
 */
std::string absolute_file_name(const std::string& filepath);

/** @brief Copy the file at source to dest
 * @param source A file to copy
 * @param dest A file (including name) of the file to create
 * @returns true iff dest exists and is a copy of source
 *
 * @note If source does not exist, dest is not created. Will overwrite an
 * existing file "dest". Dest has same permissions as source, if possible.
 */
bool copyfile(const std::string& source, const std::string& dest);

QFileInfoList getRecursiveFileInfoList(const QDir& dir = QDir(), size_t depth = 0, const QString& nameFilters = QString(), QDir::Filters filters = QDir::Files);
QStringList getRecursiveAbsoluteFilenames(const QDir& dir = QDir(), size_t depth = 0, const QString& nameFilters = QString(), const QDir::Filters& filters = QDir::Files);
QStringList getRecursiveRelativeFilenames(const QDir& dir = QDir(), size_t depth = 0, const QString& nameFilters = QString(), const QDir::Filters& filters = QDir::Files);
QStringList getRecursiveDirectories(const QDir& dir, size_t depth);
QString lastPathComponent(const QString& s);
#endif // FILEUTILS_H
