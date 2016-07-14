#ifndef CONVERT_H
#define CONVERT_H

#include <string>
#include <vector>

#include <QString>
#include <QStringList>

namespace qt
{
std::string convert(const QString&);
QString convert(const std::string&);
std::list< std::string > convert(const QStringList&);
QStringList convert(const std::vector< std::string >&);
QStringList convert(const std::list< std::string >&);
}

#endif // CONVERT_H
