#include "current_path.h"

#include <unistd.h>

namespace cpp17
{
namespace filesystem
{
path current_path()
{
	char buf[4096];

	if ( ::getcwd(buf, sizeof( buf )))
	{
		return buf;
	}

	// Dynamically allocate larger buffers until cwd fits
	std::size_t size = 2 * sizeof( buf );

	while ( true )
	{
		char* q = new char[size];

		if ( ::getcwd(q, size))
		{
			path p = q;
			delete[] q;

			return p;
		}
		else
		{
			if ( errno == ERANGE )
			{
				delete[] q;
				size *= 2;
			}
			else
			{
				delete[] q;
				break;
			}
		}
	}

	return path();
}

}
}
