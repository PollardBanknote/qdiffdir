#ifndef PBL_FS_REMOVE
#define PBL_FS_REMOVE

#include "path.h"

namespace cpp17
{
namespace filesystem
{
bool remove(const path&);
unsigned long remove_all(const path&);
}
}

#endif // REMOVE
