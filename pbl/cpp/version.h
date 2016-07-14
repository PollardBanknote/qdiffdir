/** @file version.h
 * @brief namespaces for C++11 and C++14
 */
#ifndef PBL_CPP_VERSION_H
#define PBL_CPP_VERSION_H

#if __cplusplus >= 201402L
#define CPP14
#endif
#if __cplusplus >= 201103L
#define CPP11
#endif

namespace cpp11
{
}

#ifndef CPP11
namespace cpp11
{
class nullptr_t
{
public:
	template< class T >
	operator T*() const
	{
		return 0;
	}

	template< class C, class T >
	operator T C::*( ) const
	{
		return 0;
	}
private:
	void operator&() const;
};

}

const cpp11::nullptr_t nullptr = {};
#endif // ifndef CPP11

namespace cpp14
{
}

namespace cpp17
{
}

/* cpp points to either std or cpp11/14/17 as appropriate
 */
namespace cpp
{
#ifdef CPP11
using namespace ::std;
#endif

#ifndef CPP11
using namespace ::cpp11;
#endif

#ifndef CPP14
using namespace ::cpp14;
#endif

#ifndef CPP17
using namespace ::cpp17;
#endif
}

#endif // PBL_CPP_VERSION_H
