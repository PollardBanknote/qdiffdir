#include "create_directory.h"

#include <sys/stat.h>

#include "perms.h"
#include "filestatus.h"

namespace cpp17
{
namespace filesystem
{

bool create_directory(const path& s)
{
	if ( s.empty())
	{
		return false;
	}

	return ::mkdir(s.c_str(), static_cast< int >( perms::all )) == 0;
}

bool create_directories(const path& s)
{
	path p = s.parent_path();

	if ( !p.empty())
	{
		if ( !exists(p) && !create_directories(p))
		{
			return false;
		}
	}

	return create_directory(s);
}

}
}
