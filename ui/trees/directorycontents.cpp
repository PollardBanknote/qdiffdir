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

#include "cpp/filesystem.h"
#include "util/containers.h"

namespace
{
bool is_hidden(const cpp::filesystem::path& p)
{
	const std::string s = p.filename().native();

	return !s.empty() && s[0] == '.';
}

/// @todo Turn into a find-like function
void descend(
    std::vector< std::string >&         files,
    std::vector< std::string >&         subdirs,
    const cpp::filesystem::path& path,
	unsigned             depth,
	unsigned             maxdepth
)
{
	const bool hidden_dirs  = false;
	const bool hidden_files = false;

	if ( depth < maxdepth )
	{
        subdirs.push_back(path.native());

        for ( cpp::filesystem::directory_iterator it(path), last; it != last; ++it )
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

                    files.push_back(s.substr(i));
				}
			}
		}
	}
}

std::pair< std::vector< std::string >, std::vector< std::string > > descend(
    const cpp::filesystem::path& dir,
	int         maxdepth
)
{
    std::pair< std::vector< std::string >, std::vector< std::string > > res;

	if ( maxdepth >= 0 )
	{
        descend(res.first, res.second, dir, 0, maxdepth);
	}

	return res;
}

}


std::string lastPathComponent(const std::string&);

DirectoryContents::DirectoryContents() : maxdepth(0)
{

}

std::string DirectoryContents::absolutePath() const
{
    return dir;
}

std::string DirectoryContents::absoluteFilePath(const std::string& s) const
{
    cpp::filesystem::path p(dir);
    return p / cpp::filesystem::path(s);
}

std::string DirectoryContents::relativeFilePath(const std::string& s) const
{
    const cpp::filesystem::path p(s);

    return p.lexically_relative(dir);
}

std::string DirectoryContents::name() const
{
    return cpp::filesystem::basename(dir);
}

/// @bug A relative path will change relative to the last directory, not cwd
bool DirectoryContents::cd(const std::string& path)
{
    const cpp::filesystem::path p(path);

    if ( p.is_absolute() && cpp::filesystem::is_directory(p))
	{
        dir = cpp::filesystem::cleanpath(path) + "/";
		maxdepth = 0;
		files.clear();
		subdirs.clear();
		return true;
	}

	return false;
}

std::vector< std::string > DirectoryContents::setDepth(int d)
{
	/// @todo if depth shrinks, might save some work
	if ( d != maxdepth )
	{
		maxdepth = d;
		refresh();
	}

	return getRelativeFileNames();
}

std::vector< std::string > DirectoryContents::getRelativeFileNames() const
{
	return files;
}

std::vector< std::string > DirectoryContents::getAbsoluteFileNames() const
{
    std::vector< std::string > l;

    for ( std::size_t i = 0, n = files.size(); i < n; ++i )
	{
        l.push_back(absoluteFilePath(files[i]));
	}

	return l;
}

std::vector< std::string > DirectoryContents::getDirectories() const
{
	return subdirs;
}

void DirectoryContents::refresh()
{
    const std::pair< std::vector< std::string >, std::vector< std::string > > res = descend(dir, maxdepth);

	files   = res.first;
	subdirs = res.second;
}

/// @todo Only return files from the given directory and below
DirectoryContents::update_t DirectoryContents::update(const std::string&)
{
	update_t u;

    const std::vector< std::string > old = files;

	/// @todo Only need to search from changed directory and down
	refresh();

    for ( std::size_t i = 0, n = old.size(); i < n; ++i )
	{
        if ( pbl::contains(files, old[i]))
		{
			// changed
            u.changed.push_back(old[i]);
		}
		else
		{
			// removed
            u.removed.push_back(old[i]);
		}
	}

    for ( std::size_t i = 0, n = files.size(); i < n; ++i )
	{
		// added
        if ( !pbl::contains(old, files[i]))
		{
            u.added.push_back(files[i]);
		}
	}

	return u;
}
