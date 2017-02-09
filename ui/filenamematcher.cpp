#include "filenamematcher.h"

#include "util/strings.h"

/// @todo file.bak
int FileNameMatcher::compare(
    const std::string& a,
    const std::string& b
) const
{
	if ( a == b )
	{
		return 0;
	}

	if ( a == gzalt(b))
	{
		return 1;
	}

	if ( a == cppalt(b))
	{
		return 2;
	}

	if ( a == cgalt(b))
	{
		return 3;
	}

	return -1;
}

std::string FileNameMatcher::gzalt(const std::string& s)
{
	if ( !pbl::ends_with(s, ".gz"))
	{
		return s + ".gz";
	}

	std::string t(s, 0, s.length() - 3);

	return t;
}

std::string FileNameMatcher::cppalt(const std::string& s)
{
	if ( pbl::ends_with(s, ".cpp"))
	{
		std::string t(s, 0, s.length() - 2);

		return t;
	}

	if ( pbl::ends_with(s, ".c"))
	{
		return s + "pp";
	}

	return std::string();
}

std::string FileNameMatcher::cgalt(const std::string& s)
{
	if ( pbl::ends_with(s, ".cpp"))
	{
		std::string t(s, 0, s.length() - 2);

		return t + ".gz";
	}

	if ( pbl::ends_with(s, ".c"))
	{
		return s + "pp.gz";
	}

	if ( pbl::ends_with(s, ".c.gz"))
	{
		std::string t(s, 0, s.length() - 3);

		return t + "pp";
	}

	if ( pbl::ends_with(s, ".cpp.gz"))
	{
		std::string t(s, 0, s.length() - 5);

		return t;
	}

	return std::string();
}

