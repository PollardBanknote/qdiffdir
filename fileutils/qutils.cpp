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

#include <iostream>
#include <fstream>

#ifdef _POSIX_C_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <QFileInfoList>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QRegExp>

#include <QDebug>

namespace
{
bool file_exists(const std::string& filename)
{
	return !absolute_file_name(filename).empty();
}

}

std::string absolute_file_name(const std::string& filename)
{
	if ( !filename.empty())
	{
		#if (( defined( _POSIX_VERSION ) && _POSIX_VERSION >= 200809l ) || defined( __GLIBC__ ))
		// Preferred - POSIX-2008 and glibc will allocate the path buffer
		char* res = ::realpath(filename.c_str(), NULL);

		if ( res )
		{
			std::string s = res;
			::free(res);
			return s;
		}
		else
		{
			return std::string();
		}

		#else
		#ifdef _GNU_SOURCE
		// Maybe we can rely on the GNU extension
		char* res = ::canonicalize_file_name(filename.c_str());

		if ( res )
		{
			std::string s = res;
			::free(res);
			return s;
		}

		#elif ((( defined( _POSIX_VERSION ) && _POSIX_VERSION >= 200112L ) || ( defined( _XOPEN_VERSION ) && _XOPEN_VERSION >= 500 )) && defined( PATH_MAX ))
		/// @todo PATH_MAX may be huge or -1, according to man pages for realpath
		char  resolved[PATH_MAX + 1];
		char* res = ::realpath(filename.c_str(), resolved);

		if ( res )
		{
			return resolved;
		}

		#else
		#error "No way to get absolute file path!"
		#endif // if 1
		#endif // if ( defined( _POSIX_VERSION ) && _POSIX_VERSION >= 200809l )
	}

	return std::string();
}

/** Try to do copy the file as safely as possible.
 *
 * @todo Should group permissions be copied? Both user permissions and other
 * permissions should probably be the same. But group might be a different
 * group.
 * @todo Relying so heavily on C, seems strange to use ifstream
 */
bool copyfile(
	const std::string& source,
	const std::string& dest
)
{
	// Open the input file
	std::ifstream in(source.c_str(), std::ios_base::in | std::ios_base::binary);

	if ( !in.is_open())
	{
		return false;
	}

	// change the permissions to match the original file
	mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;

	struct stat st;

	if ( ::stat(source.c_str(), &st) == 0 )
	{
		mode &= st.st_mode;
	}

	// Create dest. Possibly in place, but might need a temp file.
	std::string temp_file_name;

	int f = -1;

	if ( !file_exists(dest))
	{
		// file doesn't exist, create it directly
		/// @todo Probably some race condition here. Might prefer open with
		/// O_CREAT and falling back to the "file exists" method
		f = ::creat(dest.c_str(), S_IWUSR | S_IRUSR);
	}
	else
	{
		// file exists, so use a temp to receive the copy before overwriting
		temp_file_name = dest + "cpyXXXXXX";
		const std::size_t n = temp_file_name.length();
		char*             s = new char[n + 1];
		temp_file_name.copy(s, n, 0);
		s[n] = '\0';
		f    = ::mkstemp(s);

		if ( f != -1 )
		{
			::fchmod(f, S_IWUSR | S_IRUSR);
		}

		temp_file_name = s;
		delete[] s;
	}

	if ( f == -1 )
	{
		return false;
	}

	// Copy the contents of the file
	char buffer[4096];

	while ( in )
	{
		in.read(buffer, sizeof( buffer ));

		if ( !in.bad())
		{
			::write(f, buffer, in.gcount());
		}
		else
		{
			// error occurred. Remove the partial file
			if ( !temp_file_name.empty())
			{
				::unlink(temp_file_name.c_str());
			}
			else
			{
				::unlink(dest.c_str());
			}

			::close(f);
			return false;
		}
	}

	// Clean up the temp
	if ( !temp_file_name.empty())
	{
		// commit the new file
		int res = ::rename(temp_file_name.c_str(), dest.c_str());

		if ( res != 0 )
		{
			// Failed. Remove the temp file
			::remove(temp_file_name.c_str());

			return false;
		}
	}

	// File copy completed successfully. Fix file permissions.
	::fchmod(f, mode);
	::close(f);

	return true;
}

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
