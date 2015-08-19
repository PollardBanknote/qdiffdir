#ifndef ITEMS_H
#define ITEMS_H

#include <QString>

/** A pair of strings
 */
struct items_t
{
    items_t(
        const QString& l,
        const QString& r
    )
        : left(l), right(r)
    {

    }

    bool operator<(const items_t& o) const
    {
        return ( left.isEmpty() ? right : left ) < ( o.left.isEmpty() ? o.right : o.left );
    }

    bool operator==(const items_t& o) const
    {
        return left == o.left && right == o.right;
    }

    bool operator!=(const items_t& o) const
    {
        return left != o.left || right != o.right;
    }

    QString left;
    QString right;
};


#endif // ITEMS_H

