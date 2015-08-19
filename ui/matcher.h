#ifndef MATCHER_H
#define MATCHER_H

#include <QString>

class Matcher
{
public:
    enum {EXACT_MATCH = 0, DO_NOT_MATCH = 1000};

    virtual ~Matcher()
    {
    }

    virtual Matcher* clone() const = 0;

    // return EXACT_MATCH, DO_NOT_MATCH, or something in between indicating
    // how well the two match (lower is better)
    virtual int compare(const QString&, const QString&) const = 0;
};

class DefaultMatcher : public Matcher
{
public:
    DefaultMatcher* clone() const
    {
        return new DefaultMatcher;
    }

    int compare(
        const QString& a,
        const QString& b
    ) const
    {
        return a == b ? EXACT_MATCH : DO_NOT_MATCH;
    }

};

#endif // MATCHER_H

