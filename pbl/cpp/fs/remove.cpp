#include "remove.h"

#include <cstdio>

#include "diriter.h"

namespace cpp17
{
namespace filesystem
{
bool remove(const path& p)
{
	return ::remove(p.c_str()) == 0;
}

unsigned long remove_all(const path& p)
{
	unsigned long total = 0;

	for ( directory_iterator it(p), last; it != last; ++it )
	{
		total += remove_all(it->get_path());
	}

	if ( remove(p))
	{
		++total;
	}

	return total;
}

}
}
