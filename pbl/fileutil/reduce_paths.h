#ifndef PBL_FILEUTIL_REDUCE_PATHS_H
#define PBL_FILEUTIL_REDUCE_PATHS_H

#include <utility>
#include <string>

namespace pbl
{
namespace fs
{
/* Simplify paths as much as possible while being able to distinguish them
 */
std::pair< std::string, std::string > reduce_paths(const std::string&, const std::string&);
}
}

#endif // PBL_FILEUTIL_REDUCE_PATHS_H
