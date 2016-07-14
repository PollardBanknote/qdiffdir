/* Copyright (c) 2015, Pollard Banknote Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "file.h"

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

namespace pbl
{
namespace fs
{
file::file()
	: fd(-1), is_temp(false), filestat(0)
{

}

file::file(
	const std::string& name,
	int                flags
) : is_temp(false), filestat(0)
{
	fd = ::open(name.c_str(), flags);

	if ( fd != -1 )
	{
		filename = name;
	}
}

file::file(
	const std::string&       name,
	int                      flags,
	::cpp::filesystem::perms m
) : filename(), is_temp(false), filestat(0)
{
	fd = ::open(name.c_str(), flags, m);

	if ( fd != -1 )
	{
		filename = name;
	}
}

file::~file()
{
	delete filestat;

	if ( fd != -1 )
	{
		if ( is_temp )
		{
			remove();
		}

		::close(fd);
	}
}

bool file::is_open() const
{
	return fd != -1;
}

bool file::get_stat() const
{
	if ( filestat == 0 )
	{
		if ( fd != -1 )
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
		if ( get_stat())
		{
			return S_ISREG(filestat->st_mode) || S_ISLNK(filestat->st_mode);
		}
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

	for ( std::size_t x = 0; x < n;)
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

::cpp::filesystem::perms file::permissions() const
{
	if ( get_stat())
	{
		return static_cast< ::cpp::filesystem::perms >( filestat->st_mode & ( S_IRWXU | S_IRWXG | S_IRWXO ));
	}

	return perms::none;
}

void file::chmod(::cpp::filesystem::perms m)
{
	if ( fd != -1 )
	{
		::fchmod(fd, m);
	}
}

void file::chmod(const file& other)
{
	if ( fd != -1 && other.fd != -1 )
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

		if ( fd != -1 )
		{
			is_temp  = true;
			filename = temp_file_name;
			::fchmod(fd, S_IWUSR | S_IRUSR);

			return true;
		}
	}

	return false;
}

bool file::rename(const std::string& dest)
{
	if ( fd != -1 )
	{
		const int res = std::rename(filename.c_str(), dest.c_str());

		if ( res == 0 )
		{
			filename = dest;

			return true;
		}
	}

	return false;
}

bool file::realize(const std::string& dest)
{
	if ( fd != -1 && is_temp )
	{
		if ( rename(dest))
		{
			is_temp = false;

			return true;
		}
	}

	return false;
}

/// @bug Should probably lseek to beginning and ftruncate
/// @todo Use sendfile(2) for efficiency
bool file::copy(file& in)
{
	if ( fd != -1 && in.fd != -1 )
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

			if ( write(buffer, n) == -1 )
			{
				return false;
			}
		}

		// flush to disk
		flush();

		return true;
	}

	return false;
}

/** @todo See FIO19-C from cert for problems with getting file size
 */
file::size_type file::size() const
{
	if ( get_stat())
	{
		return filestat->st_size;
	}

	return 0;
}

class file::seek_guard
{
public:
	explicit seek_guard(const file& f) : fd(-1), position(-1)
	{
		if ( f.is_open())
		{
			position = ::lseek(f.fd, 0, SEEK_CUR);

			if ( position != off_t(-1))
			{
				fd = f.fd;
			}
		}
	}

	~seek_guard()
	{
		if ( fd != -1 )
		{
			::lseek(fd, position, SEEK_SET);
		}
	}

private:
	int   fd;
	off_t position;
};

/// @todo seek guard
int file::compare(const file& other)
{
	if ( !is_open() || !other.is_open())
	{
		return -1;
	}

	// files of different size are obviously different
	if ( size() != other.size())
	{
		return 0;
	}

	// files with the same dev/inode are obviously the same and don't need to
	// be compared
	if ( get_stat() && other.get_stat() && filestat->st_ino == other.filestat->st_ino && filestat->st_dev == other.filestat->st_dev )
	{
		return 1;
	}

	// Save the current file position
	seek_guard g1(*this);
	seek_guard g2(other);

	// go to the start of each file
	if ( ::lseek(fd, 0, SEEK_SET) == off_t(-1))
	{
		return -1;
	}

	if ( ::lseek(other.fd, 0, SEEK_SET) == off_t(-1))
	{
		return -1;
	}

	// Start reading buffers
	char buf1[4096];
	char buf2[4096];

	std::size_t size1 = 0;
	std::size_t size2 = 0;

	bool eof1 = false;
	bool eof2 = false;

	while ( true )
	{
		// read from each file
		if ( !eof1 && size1 < sizeof( buf1 ))
		{
			const ssize_t n1 = ::read(fd, buf1 + size1, sizeof( buf1 ) - size1);

			if ( n1 == -1 )
			{
				return -1;
			}

			if ( n1 == 0 )
			{
				eof1 = true;
			}

			size1 += n1;
		}

		if ( !eof2 && size2 < sizeof( buf2 ))
		{
			const ssize_t n2 = ::read(other.fd, buf2 + size2, sizeof( buf2 ) - size2);

			if ( n2 == -1 )
			{
				return -1;
			}

			if ( n2 == 0 )
			{
				eof2 = true;
			}

			size2 += n2;
		}

		// Compare files based on what we have
		const std::size_t m = std::min(size1, size2);

		// files are different
		if ( std::memcmp(buf1, buf2, m) != 0 )
		{
			return 0;
		}
		else
		{
			// files are the same
			if ( eof1 && eof2 )
			{
				return 1;
			}

			// files are the same so far... save the data
			if ( size1 > m )
			{
				std::memmove(buf1, buf1 + m, size1 - m);
			}

			size1 -= m;

			if ( size2 > m )
			{
				std::memmove(buf2, buf2 + m, size2 - m);
			}

			size2 -= m;

			// files have different size
			if (( eof1 && size2 != 0 ) || ( eof2 && size1 != 0 ))
			{
				return 0;
			}
		}
	}

	return -1;

}

}
}
