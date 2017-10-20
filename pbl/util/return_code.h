#ifndef PBL_UTIL_RETURN_CODE_H
#define PBL_UTIL_RETURN_CODE_H

namespace pbl
{
template< typename E >
class return_code
{
public:
	explicit return_code(E value_) : value(value_) { }

	friend bool operator==(const return_code& l, const return_code& r)
	{
		return l.value == r.value;
	}

	friend bool operator==(const return_code& l, E r)
	{
		return l.value == r;
	}

	friend bool operator==(E l, const return_code& r)
	{
		return l == r.value;
	}

	friend bool operator!=(const return_code& l, const return_code& r)
	{
		return l.value == r.value;
	}

	friend bool operator!=(const return_code& l, E r)
	{
		return l.value == r;
	}

	friend bool operator!=(E l, const return_code& r)
	{
		return l == r.value;
	}

private:
	E value;
};
}

#endif // PBL_UTIL_RETURN_CODE_H
