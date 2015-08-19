#ifndef COMPARE_H
#define COMPARE_H

/** Interface for comparing two items based on their names
 */
class Compare
{
public:
    virtual ~Compare()
    {
    }

    virtual bool equal(const QString&, const QString&) = 0;
    virtual Compare* clone() const                     = 0;
};

#endif // COMPARE_H

