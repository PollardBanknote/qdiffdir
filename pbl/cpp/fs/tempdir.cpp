#include "tempdir.h"

#include <cstdlib>

namespace cpp17
{
namespace filesystem
{

/* TMPDIR is the POSIX variable for the temporary directory. Other variables
 * are sometimes used, however, in non-conforming environments. Those are
 * checked as fallback (following Boost's implementation)
 */
path temp_directory_path()
{
	const char* p = std::getenv("TMPDIR");

	if ( !p )
	{
		p = std::getenv("TMP");

		if ( !p )
		{
			p = std::getenv("TEMP");

			if ( !p )
			{
				p = std::getenv("TEMPDIR");

				if ( !p )
				{
					p = "/tmp";
				}
			}
		}
	}

	return path(p);
}

}
}
