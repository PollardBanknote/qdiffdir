#include "directorycontents.h"

#include <algorithm>
#include <climits>
#include "cpp/filesystem.h"

#include "util/strings.h"

namespace
{
bool is_hidden(const cpp::filesystem::path& p)
{
	const std::string s = p.filename().native();

	return !s.empty() && s[0] == '.';
}

bool is_absolute(const std::string& s)
{
	return !s.empty() && s[0] == '/';
}

}

void DirectoryContents::swap(DirectoryContents& n)
{
	name_.swap(n.name_);
	children.swap(n.children);
	files.swap(n.files);
}

bool DirectoryContents::valid() const
{
	return !name_.empty();
}

// Need to populate this directory's contents
void DirectoryContents::init(const std::string& path)
{
	// Get all directories and children
	std::vector< std::string > dirs;
	std::vector< std::string > filenames;

	const bool hidden_dirs  = false;
	const bool hidden_files = false;

	for ( cpp::filesystem::directory_iterator it(path), last; it != last; ++it )
	{
		cpp::filesystem::file_status s = it->status();

		if ( cpp::filesystem::is_directory(s))
		{
			if ( hidden_dirs || !is_hidden(it->path()))
			{
				const cpp::filesystem::path rel = it->path().lexically_relative(path);
				dirs.push_back(rel.native());
			}
		}
		else if ( cpp::filesystem::is_regular_file(s) || cpp::filesystem::is_symlink(s))
		{
			if ( hidden_files || !is_hidden(it->path()))
			{
				const cpp::filesystem::path rel = it->path().lexically_relative(path);
				filenames.push_back(rel.native());
			}
		}
	}

	// Save
	std::sort(filenames.begin(), filenames.end());
	files.swap(filenames);

	std::sort(dirs.begin(), dirs.end());
	children.resize(dirs.size());

	for ( std::size_t i = 0, n = dirs.size(); i < n; ++i )
	{
		children[i].name_.swap(dirs[i]);
	}
}

void DirectoryContents::change_depth()
{
	change_depth(INT_MAX);
}

void DirectoryContents::change_depth(int d)
{
	if ( name_.empty())
	{
		children.clear();
		files.clear();
	}
	else
	{
		change_depth(name_, 0, d);
	}
}

void DirectoryContents::change_depth(
	const std::string& current_path,
	int                current_depth,
	int                d
)
{
	if ( current_depth < d )
	{
		if ( files.empty() && children.empty())
		{
			init(current_path);
		}

		// Move down a level
		for ( std::size_t i = 0, n = children.size(); i < n; ++i )
		{
			children[i].change_depth(current_path + "/" + children[i].name_, current_depth + 1, d);
		}
	}
	else
	{
		children.clear();
		files.clear();
	}
}

bool DirectoryContents::change_root(
	const std::string& dir,
	int                d
)
{
	if ( !dir.empty())
	{
		name_ = ( is_absolute(dir) && cpp::filesystem::is_directory(dir))
		        ? cpp::filesystem::cleanpath(dir)
				: std::string();
		children.clear();
		files.clear();

		change_depth(d);

		return true;
	}

	return false;
}

// dirname begins with current_path + "/"
bool DirectoryContents::rescan(
	const std::string& current_path,
	const std::string& dirname,
	int                depth,
	int                maxdepth
)
{
	/// @todo binary search because children is sorted
	for ( std::size_t i = 0, n = children.size(); i < n; ++i )
	{
		const std::string s = current_path + "/" + children[i].name_;

		if ( dirname == s )
		{
			// Found the dir
			if ( cpp::filesystem::is_directory(s))
			{
				children[i].children.clear();
				children[i].files.clear();
				children[i].change_depth(s, depth + 1, maxdepth);
			}
			else
			{
				// no longer exists on disk
				children.erase(children.begin() + i);
			}

			return true;
		}
		else if ( pbl::starts_with(dirname, s + "/"))
		{

			// Descend
			return children[i].rescan(s, dirname, depth + 1, maxdepth);
		}
	}

	return false;
}

void DirectoryContents::rescan(
	const std::string& dirname,
	int                maxdepth
)
{
	if ( !name_.empty())
	{
		if ( name_ == dirname )
		{
			// root has changed
			children.clear();
			files.clear();

			if ( cpp::filesystem::is_directory(dirname))
			{
				change_depth(maxdepth);
			}
			else
			{
				// directory doesn't exist anymore
				name_.clear();
			}
		}
		else
		{
			if ( pbl::starts_with(dirname, name_ + "/"))
			{
				rescan(name_, dirname, 0, maxdepth);
			}
		}
	}
}

std::size_t DirectoryContents::dircount() const
{
	return children.size();
}

std::size_t DirectoryContents::filecount() const
{
	return files.size();
}

const DirectoryContents& DirectoryContents::subdir(std::size_t i) const
{
	return children[i];
}

const std::string& DirectoryContents::filename(std::size_t i) const
{
	return files[i];
}

const std::string& DirectoryContents::name() const
{
	return name_;
}
