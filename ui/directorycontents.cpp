#include "directorycontents.h"

#include <algorithm>
#include "cpp/filesystem.h"

namespace
{
bool is_hidden(const cpp::filesystem::path& p)
{
	const std::string s = p.filename().native();

	return !s.empty() && s[0] == '.';
}
}

void DirectoryContents::swap(DirectoryContents& n)
{
	name.swap(n.name);
	children.swap(n.children);
	files.swap(n.files);
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
		children[i].name.swap(dirs[i]);
	}

}
