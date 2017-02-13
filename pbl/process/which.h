#ifndef PBL_PROCESS_WHICH_H
#define PBL_PROCESS_WHICH_H

#include <string>

namespace pbl
{
/** Searches PATH for filename
 * @param filename The name of an executable (cannot contain a slash)
 *
 * If filename exists in some dir in PATH, returns full path to that file.
 * Returns empty string if not found or error
 */
std::string which(const std::string& filename);
}

#endif // PBL_PROCESS_WHICH_H
