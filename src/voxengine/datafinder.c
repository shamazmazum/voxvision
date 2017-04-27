#include <sys/param.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <strings.h>

#include <lua.h>

#include <modules.h>
#include "engine.h"

static int check_file (const char* filename)
{
    struct stat sb;

    if (!stat (filename, &sb) && S_ISREG(sb.st_mode)) return 1;
    return 0;
}

int vox_find_data_file (const char *filename, char *fullpath)
{
    // At first, try to find data file in system-wide data directory
    strlcpy (fullpath, VOX_DATA_PATH, MAXPATHLEN);
    strlcat (fullpath, filename, MAXPATHLEN);
    if (check_file (fullpath)) return 1;

    // Then check at ~/.voxvision
    strlcpy (fullpath, getenv("HOME"), MAXPATHLEN);
    strlcat (fullpath, "/.voxvision/", MAXPATHLEN);
    strlcat (fullpath, filename, MAXPATHLEN);
    if (check_file (fullpath)) return 1;

    // Then as last resort try environment variable VOXVISION_DATA
    strlcpy (fullpath, getenv("VOXVISION_DATA"), MAXPATHLEN);
    strlcat (fullpath, "/", MAXPATHLEN);
    strlcat (fullpath, filename, MAXPATHLEN);
    if (check_file (fullpath)) return 1;

    return 0;
}
