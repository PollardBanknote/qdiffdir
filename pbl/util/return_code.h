#ifndef PBL_UTIL_RETURN_CODE_H
#define PBL_UTIL_RETURN_CODE_H

#include <iostream>

namespace pbl
{
/** Wrapper around an enum E that prevents unwanted conversions (ex., to bool)
 */
template< typename E >
class return_code
{
public:
	return_code(E value_)
		: value(value_)
	{
	}

	friend bool operator==(
		const return_code& l,
		const return_code& r
	)
	{
		return l.value == r.value;
	}

	friend bool operator!=(
		const return_code& l,
		const return_code& r
	)
	{
		return l.value != r.value;
	}

	friend std::ostream& operator<<(
		std::ostream&      os,
		const return_code& c
	)
	{
		return os << c.value;
	}

private:
	E value;
};
}

#endif // PBL_UTIL_RETURN_CODE_H
