#include "reduce_paths.h"

#include "cpp/filesystem.h"

namespace
{
void replace_intermediate_paths(std::string& s)
{
	const std::size_t i = s.find_first_of('/');

	if ( i != std::string::npos )
	{
		const std::size_t j = s.find_last_of('/');

		if ( j > i )
		{
			s.replace(i + 1, j - i - 1, "...");
		}
	}
}
}

namespace pbl
{
namespace fs
{
std::pair<std::string, std::string> reduce_paths(const std::string& l_, const std::string& r_)
{
	std::string l = cpp::filesystem::basename(l_);
	std::string r = cpp::filesystem::basename(r_);

	if ( l == r )
	{
		l = l_;
		r = r_;

		// Find common ancestor
		std::size_t       i = 0;
		const std::size_t n = std::min(l.length(), r.length());

		while ( i < n && l[i] == r[i] )
		{
			++i;
		}

		while ( i > 0 && l[i - 1] != '/' )
		{
			--i;
		}

		// Erase common ancestor
		l.erase(0, i);
		r.erase(0, i);

		// Remove intermediate directories
		replace_intermediate_paths(l);
		replace_intermediate_paths(r);
	}

	return std::make_pair(l, r);
}
}
}
