#ifndef PBL_FS_CREATE_DIRECTORY_H
#define PBL_FS_CREATE_DIRECTORY_H

#include "path.h"

namespace cpp17
{
namespace filesystem
{
bool create_directory(const path&);

bool create_directories(const path&);
}
}

#endif // PBL_FS_CREATE_DIRECTORY_H
