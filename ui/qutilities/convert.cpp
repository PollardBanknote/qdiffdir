#include "convert.h"

namespace qt
{
std::string convert(const QString& s)
{
    return s.toStdString();
}

QString convert(const std::string& s)
{
    return QString::fromStdString(s);
}

std::list< std::string > convert(const QStringList& l)
{
    std::list< std::string > r;
    for (int i = 0; i < l.count(); ++i)
        r.push_back(convert(l.at(i)));
    return r;
}

QStringList convert(const std::vector< std::string >& v)
{
    QStringList r;
    for (std::size_t i = 0; i < v.size(); ++i)
        r << convert(v[i]);
    return r;
}

QStringList convert(const std::list< std::string >& l)
{
    QStringList r;

    for (std::list< std::string >::const_iterator it = l.begin(), last = l.end(); it != last; ++it)
        r << convert(*it);

    return r;
}
}
