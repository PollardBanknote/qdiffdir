#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <set>
#include <iterator>

namespace pbl
{
template< typename Iterator, typename T >
bool contains(Iterator first, Iterator last, const T& value)
{
    while (first != last)
    {
        if (*first == value)
            return true;
        ++first;
    }
    return false;
}

template< typename Container, typename U >
bool contains(const Container& c, const U& value)
{
    return contains(c.begin(), c.end(), value);
}

template< typename Container >
std::set< typename Container::value_type > make_set(const Container& c)
{
    std::set< typename Container::value_type > s;

    s.insert(c.begin(), c.end());
    return s;
}
}

#endif // CONTAINERS_H
