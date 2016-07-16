#ifndef CPP_FILESYSTEM_H
#define CPP_FILESYSTEM_H

#include "version.h"

#ifdef CPP17
#include <filesystem.h>
#else
#include "fs/absolute.h"
#include "fs/basename.h"
#include "fs/cleanpath.h"
#include "fs/copyfile.h"
#include "fs/create_directory.h"
#include "fs/current_path.h"
#include "fs/direntry.h"
#include "fs/diriter.h"
#include "fs/filestatus.h"
#include "fs/filetype.h"
#include "fs/path.h"
#include "fs/perms.h"
#include "fs/remove.h"
#include "fs/tempdir.h"
#endif

#endif // FILESYSTEM_H
