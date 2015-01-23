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
#include "fileutils/qutils.h"

#include <QFileInfoList>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QRegExp>

#include <QDebug>

#include "fileutils/fileutils.h"

namespace
{
QStringList parseNameFilters(QString filters)
{
	QStringList parsed_filters;
	QRegExp     regexp(".*\\((.*)\\)");

	if ( regexp.exactMatch(filters))
	{
		filters = regexp.cap(1);
	}

	QStringList token = filters.split(';', QString::SkipEmptyParts);

	for ( int i = 0; i < token.count(); ++i )
	{
		parsed_filters << token.at(i).trimmed();
	}

	return parsed_filters;
}

}

QFileInfoList getRecursiveFileInfoList(
	const QDir&    dir,
	size_t         depth,
	const QString& nameFilters,
	QDir::Filters  filters
)
{
	// remove the . and .. folders
	filters |= QDir::NoDotAndDotDot;

	QFileInfoList files;

	if ( nameFilters.isEmpty())
	{
		files = dir.entryInfoList(filters);
	}
	else
	{
		files = dir.entryInfoList(parseNameFilters(nameFilters), filters);
	}

	if ( depth > 0 )
	{
		QFileInfoList directory_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

		foreach(QFileInfo dir_info, directory_list)
		{
			files.append(getRecursiveFileInfoList(dir_info.absoluteFilePath(), depth - 1, nameFilters, filters));
		}
	}

	return files;
}

QStringList getRecursiveAbsoluteFilenames(
	const QDir&          dir,
	size_t               depth,
	const QString&       nameFilters,
	const QDir::Filters& filters
)
{
	QStringList absolute_filenames;

	foreach(QFileInfo file_info, getRecursiveFileInfoList(dir, depth, nameFilters, filters))
	{
		QString filename = file_info.absoluteFilePath();

		if ( file_info.isDir())
		{
			filename += QDir::separator();
		}

		absolute_filenames.append(filename);
	}
	return absolute_filenames;
}

QStringList getRecursiveRelativeFilenames(
	const QDir&          dir,
	size_t               depth,
	const QString&       nameFilters,
	const QDir::Filters& filters
)
{
	QStringList relative_filenames;

	foreach(QFileInfo file_info, getRecursiveFileInfoList(dir, depth, nameFilters, filters))
	{
		QString filename = dir.relativeFilePath(file_info.absoluteFilePath());

		if ( file_info.isDir())
		{
			filename += QDir::separator();
		}

		relative_filenames.append(filename);
	}
	return relative_filenames;
}

QStringList getRecursiveDirectories(
	const QDir& dir,
	size_t      depth
)
{
	QStringList l;

	l << dir.absolutePath()
	  << getRecursiveAbsoluteFilenames(dir, depth, QString(), QDir::Dirs);

	return l;
}

QString lastPathComponent(const QString& s)
{
	int i = s.lastIndexOf('/');

	// no slash, whole thing is the last component
	if ( i == -1 )
	{
		return s;
	}

	if ( i < s.length() - 1 )
	{
		// string has the form "path/to/file"
		return s.mid(i + 1);
	}

	// skip trailing slash
	while ( i > 0 && s[i - 1] == '/' )
	{
		--i;
	}

	if ( i == 0 )
	{
		// path consists entirely of the / character
		return QString();
	}

	int j = s.lastIndexOf('/', i - 1);

	if ( j == -1 )
	{
		// string has the form "dir/"
		return s.left(i);
	}

	// string has the form "path/to/dir/"
	return s.mid(j + 1, i - ( j + 1 ));
}
