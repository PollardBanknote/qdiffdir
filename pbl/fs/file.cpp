#include "file.h"
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace pbl
{
namespace fs
{
file::file()
    :fd(-1), is_temp(false), filestat(0)
{

}

file::file(
    const std::string& name,
    int                flags
) : is_temp(false), filestat(0)
{
	fd = ::open(name.c_str(), flags);
    if (fd != -1)
        filename = name;
}

file::file(
    const std::string& name,
    int                flags,
    pbl::fs::perms     m
) : filename(), is_temp(false), filestat(0)
{
    fd = ::open(name.c_str(), flags, m);
    if (fd != -1)
        filename = name;
}

file::~file()
{
    delete filestat;

	if ( fd != -1 )
	{
        if (is_temp)
            remove();
		::close(fd);
	}
}

bool file::is_open() const
{
	return fd != -1;
}

bool file::get_stat() const
{
    if (filestat == 0)
    {
        if (fd != -1)
        {
            struct stat* p = new struct stat;

            ::fstat(fd, p);
            filestat = p;
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool file::is_file() const
{
	if ( fd != -1 )
	{
        if (get_stat())
            return S_ISREG(filestat->st_mode) || S_ISLNK(filestat->st_mode);
	}

	return false;
}

file::size_type file::read(
	char*       buffer,
	std::size_t n
)
{
	if ( fd == -1 )
	{
		return -1;
	}

	return ::read(fd, buffer, n);
}

file::size_type file::write(
	const char* buffer,
	std::size_t n
)
{
	if ( fd == -1 )
	{
		return -1;
    }

    for (std::size_t x = 0; x < n;)
    {
        const size_type m = ::write(fd, buffer + x, n - x);

        if ( m == -1 )
        {
            // error
            return -1;
        }

        x += m;
    }

    return n;
}

void file::flush()
{
	if ( fd != -1 )
	{
		::fsync(fd);
	}
}

pbl::fs::perms file::permissions() const
{
    if (get_stat())
        return static_cast<pbl::fs::perms>(filestat->st_mode & ( S_IRWXU | S_IRWXG | S_IRWXO ));

    return perms::none;
}

void file::chmod(pbl::fs::perms m)
{
	if ( fd != -1 )
	{
		::fchmod(fd, m);
	}
}

void file::chmod(const file & other)
{
    if (fd != -1 && other.fd != -1)
    {
        ::fchmod(fd, other.permissions());
    }
}

void file::remove()
{
	if ( fd != -1 )
	{
		::unlink(filename.c_str());
	}
}

/**
 * @todo Linux 3.11 supports O_TMPFILE - an anonymous file that can be "made
 * real". See man open.
 */
bool file::mkstemp(const std::string& name)
{
	if ( fd == -1 )
	{
		// file already exists, so copy to a temp first then overwrite atomically
		std::string       temp_file_name = name + "cpyXXXXXX";
		const std::size_t n              = temp_file_name.length();
		char*             s              = new char[n + 1];
		temp_file_name.copy(s, n, 0);
		s[n]           = '\0';
		fd             = ::mkstemp(s);
		temp_file_name = s;
		delete[] s;
        if (fd != -1)
        {
            is_temp = true;
            filename = temp_file_name;
            ::fchmod(fd, S_IWUSR | S_IRUSR);
            return true;
        }
	}

	return false;
}

bool file::rename(const std::string &dest)
{
    if (fd != -1)
    {
        const int res = ::rename(filename.c_str(), dest.c_str());
        if (res == 0)
        {
            filename = dest;
            return true;
        }
    }
    return false;
}

bool file::realize(const std::string &dest)
{
    if (fd != -1 && is_temp)
    {
        if (rename(dest))
        {
            is_temp = false;
            return true;
        }
    }
    return false;
}

/// @bug Should probably lseek to beginning and ftruncate
bool file::copy(file &in)
{
    if (fd != -1 && in.fd != -1)
    {
        // Copy the contents of the file
        char buffer[4096];

        while ( true )
        {
            const size_type n = in.read(buffer, sizeof( buffer ));

            if ( n < 0 )
            {
                // error
                return false;
            }

            // eof
            if ( n == 0 )
            {
                break;
            }

            if (write(buffer, n) == -1)
                return false;
        }

        // flush to disk
        flush();
        return true;
    }
    return false;
}

}
}
