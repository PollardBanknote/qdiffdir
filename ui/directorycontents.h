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

#include <QString>
#include <QDir>

/** Used to observe the files/subdirectories of a directory
 *
 * @todo Remove all traces of Qt
 */
class DirectoryContents
{
public:
	DirectoryContents();

	QString absolutePath() const;

	QString absoluteFilePath(const QString& s) const;

	QString relativeFilePath(const QString& s) const;

	QString name() const;

	bool cd(const QString& path);

	void setDepth(int d);

	QStringList getRelativeFileNames() const;

	QStringList getDirectories() const;

	QStringList getAbsoluteFileNames() const;

	void refresh();
private:
	void descend(const QString& path, const QString& rel, int depth);

	QDir dir;

	int maxdepth;

	// relative paths of each file
	QStringList files;

	// absolute paths of directories that we should watch
	QStringList subdirs;
};


#endif // DIRECTORYCONTENTS_H
