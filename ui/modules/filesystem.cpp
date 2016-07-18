#include "filesystem.h"

#include "cpp/filesystem.h"

namespace
{
bool is_hidden(const cpp::filesystem::path& p)
{
	const std::string s = p.filename().native();

	return !s.empty() && s[0] == '.';
}

}

std::pair< std::vector< std::string >, std::vector< std::string > > FileSystem::contents(const std::string& path)
{
	std::pair< std::vector< std::string >, std::vector< std::string > > res;

	const bool hidden_dirs  = false;
	const bool hidden_files = false;

	for ( cpp::filesystem::directory_iterator it(path), last; it != last; ++it )
	{
		cpp::filesystem::file_status s = it->status();

		if ( cpp::filesystem::is_directory(s))
		{
			if ( hidden_dirs || !is_hidden(it->get_path()))
			{
				const cpp::filesystem::path rel = it->get_path().lexically_relative(path);
				res.second.push_back(rel.native());
			}
		}
		else if ( cpp::filesystem::is_regular_file(s) || cpp::filesystem::is_symlink(s))
		{
			if ( hidden_files || !is_hidden(it->get_path()))
			{
				const cpp::filesystem::path rel = it->get_path().lexically_relative(path);
				res.first.push_back(rel.native());
			}
		}
	}

	return res;
}

bool FileSystem::exists(const std::string& s)
{
	return cpp::filesystem::exists(s);
}

bool FileSystem::copy(
	const std::string& from,
	const std::string& to
)
{
	return cpp::filesystem::copy_file(from, to, copy_options::overwrite_existing);
}

bool FileSystem::is_directory(const std::string& s)
{
	return cpp::filesystem::is_directory(s);
}

bool FileSystem::is_absolute(const std::string& s)
{
	return !s.empty() && s[0] == '/';
}
