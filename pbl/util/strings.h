#ifndef STRINGS_H
#define STRINGS_H

#include <string>

namespace pbl
{
bool starts_with(const std::string& s, const std::string& head);
bool ends_with(const std::string& s, const std::string& tail);
}
#endif // STRINGS_H
