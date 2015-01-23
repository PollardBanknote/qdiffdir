
#include <string>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
/** Do two files have different sizes
 *
 * Only returns true if file sizes could be correctly determined. If
 * a file is inaccessible, for example, function will return false
 *
 * See FIO19-C from cert for problems with getting file size
 */
bool different_sizes(const std::string& f, const std::string& g)
{
	struct stat buf1;
	struct stat buf2;

	if (stat(f.c_str(), &buf1) == 0 && stat(g.c_str(), &buf2) == 0)
	{
		return buf1.st_size != buf2.st_size;
	}
	return false;
}
}

namespace pbl
{
namespace file
{
int compare(const std::string& f, const std::string& g)
{
	if (different_sizes(f, g))
		return 0;

	FILE* f1 = fopen(f.c_str(), "rb");
	if (!f1)
	{
		return -1;
	}

	FILE* f2 = fopen(g.c_str(), "rb");
	if (!f2)
	{
		fclose(f1);
		return -1;
	}

	char buf1[4096];
	char buf2[4096];

	int result = 1;

	while (true)
	{
		const size_t n1 = fread(buf1, 1, sizeof(buf1), f1);
		const bool eof1 = (n1 != sizeof(buf1));
		if (eof1 && ferror(f1))
		{
			result = -1;
			break;
		}
		const size_t n2 = fread(buf2, 1, sizeof(buf2), f2);
		const bool eof2 = (n2 != sizeof(buf2));
		if (eof2 && ferror(f2))
		{
			result = -1;
			break;
		}

		if (n1 != n2 || memcmp(buf1, buf2, n1) != 0)
		{
			result = 0;
			break;
		}

		if (eof1 && eof2)
			break;
	}

	fclose(f2);
	fclose(f1);
	return result;
}

}
}
