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
#include "directorycontents.h"

#include "fs/diriter.h"

namespace
{
bool is_hidden(const pbl::fs::path& p)
{
	const std::string s = p.filename().native();

	return !s.empty() && s[0] == '.';
}

/// @todo Turn into a find-like function
void descend(
	QStringList&         files,
	QStringList&         subdirs,
	const pbl::fs::path& path,
	unsigned             depth,
	unsigned             maxdepth
)
{
	const bool hidden_dirs  = false;
	const bool hidden_files = false;

	if ( depth < maxdepth )
	{
		subdirs << QString::fromStdString(path.native());

		for ( pbl::fs::directory_iterator it(path), last; it != last; ++it )
		{
			if ( it->status().type() == file_type::directory )
			{
				if ( hidden_dirs || !is_hidden(it->get_path()))
				{
					descend(files, subdirs, it->get_path(), depth + 1, maxdepth);
				}
			}
			else
			{
				if ( hidden_files || !is_hidden(it->get_path()))
				{
					std::string s = it->get_path().native();

					// keep only last (depth + 1) components
					// points to first character of last (depth + 1) components
					std::size_t i = std::string::npos;

					for ( unsigned n = 0; n < depth + 1; ++n )
					{
						std::size_t k = std::string::npos;

						if ( i != std::string::npos )
						{
							k = s.find_last_not_of('/', i - 1);

							if ( k == std::string::npos )
							{
								i = 0;
								break;
							}
						}

						std::size_t j = s.find_last_of('/', k);

						if ( j == std::string::npos )
						{
							i = 0;
							break;
						}

						i = j + 1;
					}

					files << QString::fromStdString(s.substr(i));
				}
			}
		}
	}
}

std::pair< QStringList, QStringList > descend(
	const QDir& dir,
	int         maxdepth
)
{
	std::pair< QStringList, QStringList > res;

	if ( maxdepth >= 0 )
	{
		pbl::fs::path p(dir.absolutePath().toStdString());

		descend(res.first, res.second, p, 0, maxdepth);
	}

	return res;
}

}


QString lastPathComponent(const QString&);

DirectoryContents::DirectoryContents() : maxdepth(0)
{

}

QString DirectoryContents::absolutePath() const
{
	QString s = QDir::cleanPath(dir.absolutePath());

	if ( !s.endsWith(QDir::separator()))
	{
		s += QDir::separator();
	}

	return s;
}

QString DirectoryContents::absoluteFilePath(const QString& s) const
{
	return dir.absoluteFilePath(s);
}

QString DirectoryContents::relativeFilePath(const QString& s) const
{
	return dir.relativeFilePath(s);
}

QString DirectoryContents::name() const
{
	return lastPathComponent(dir.absolutePath());
}

/// @bug A relative path will change relative to the last directory, not cwd
bool DirectoryContents::cd(const QString& path)
{
	if ( dir.cd(path))
	{
		maxdepth = 0;
		files.clear();
		subdirs.clear();
		return true;
	}

	return false;
}

QStringList DirectoryContents::setDepth(int d)
{
	/// @todo if depth shrinks, might save some work
	if ( d != maxdepth )
	{
		maxdepth = d;
		refresh();
	}

	return getRelativeFileNames();
}

QStringList DirectoryContents::getRelativeFileNames() const
{
	return files;
}

QStringList DirectoryContents::getAbsoluteFileNames() const
{
	QStringList l;

	for ( int i = 0, n = files.count(); i < n; ++i )
	{
		l << absoluteFilePath(files.at(i));
	}

	return l;
}

QStringList DirectoryContents::getDirectories() const
{
	return subdirs;
}

void DirectoryContents::refresh()
{
	const std::pair< QStringList, QStringList > res = descend(dir, maxdepth);

	files   = res.first;
	subdirs = res.second;
}

/// @todo Only return files from the given directory and below
DirectoryContents::update_t DirectoryContents::update(const QString&)
{
	update_t u;

	const QStringList old = files;

	/// @todo Only need to search from changed directory and down
	refresh();

	for ( int i = 0, n = old.count(); i < n; ++i )
	{
		if ( files.contains(old.at(i)))
		{
			// changed
			u.changed << old.at(i);
		}
		else
		{
			// removed
			u.removed << old.at(i);
		}
	}

	for ( int i = 0, n = files.count(); i < n; ++i )
	{
		// added
		if ( !old.contains(files.at(i)))
		{
			u.added << files.at(i);
		}
	}

	return u;
}
