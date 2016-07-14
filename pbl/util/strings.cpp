#include "strings.h"

namespace pbl
{
bool starts_with(
	const std::string& s,
	const std::string& head
)
{
	const std::size_t n = s.length();
	const std::size_t m = head.length();

	if ( n >= m )
	{
		return s.compare(0, m, head) == 0;
	}

	return false;
}

bool ends_with(
	const std::string& s,
	const std::string& tail
)
{
	const std::size_t n = s.length();
	const std::size_t m = tail.length();

	if ( n >= m )
	{
		return s.compare(n - m, m, tail) == 0;
	}

	return false;
}

}
