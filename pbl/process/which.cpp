#include "which.h"

#include <cstdlib>
#include <cerrno>

#include <unistd.h>

namespace
{
/// @todo Move to system/user.cpp ?
std::string getpath()
{
	std::string s;

	if ( const char* path = std::getenv("PATH"))
	{
		s = path;
	}

	return s;
}

// file exists and can be executed
bool is_executable(const std::string& path)
{
	return ::access(path.c_str(), X_OK) == 0;
}

std::string myrealpath(const std::string& path)
{
	std::string s;

	// relative path
	#if (( defined( _POSIX_VERSION ) && _POSIX_VERSION >= 200809l ) || defined( __GLIBC__ ))

	if ( char* res = ::realpath(path.c_str(), NULL))
	{
		s = res;
		::free(res);
	}

	#else
	#error "Need to resolve relative path"
	#endif

	return s;
}

}

namespace pbl
{
std::string which(const std::string& filename)
{
	// Explicitly pathed executables are not searched
	if ( filename.find('/') != std::string::npos )
	{
		if ( filename[0] == '/' )
		{
			// absolute path
			if ( is_executable(filename))
			{
				return filename;
			}
		}
		else
		{
			std::string abs = myrealpath(filename);

			if ( is_executable(abs))
			{
				return abs;
			}
		}
	}
	else if ( !filename.empty())
	{
		/// @todo Provide an iterator over the parts of the string
		const std::string path = getpath();
		const std::size_t n    = path.length();

		for ( std::size_t i = 0; i < n;)
		{
			const std::size_t j = path.find(':', i);

			const std::size_t len = ( j != std::string::npos ? j - i : n - i );

			std::string prefix;

			if ( len != 0 )
			{
				prefix.assign(path, i, len);
			}
			else
			{
				prefix = ".";
			}

			const std::string full = prefix + "/" + filename;

			if ( is_executable(full))
			{
				return full;
			}

			i = i + len + 1;
		}

		// PATH may end with a colon, meaning cwd
		if ( n != 0 && path[n - 1] == ':' )
		{
			const std::string full = "./" + filename;

			if ( is_executable(full))
			{
				return full;
			}
		}
	}

	return std::string();
}

}
