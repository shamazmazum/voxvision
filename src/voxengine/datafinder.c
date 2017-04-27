#include <sys/stat.h>
#include <stdlib.h>
#include <strings.h>

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
    strcpy (fullpath, VOX_DATA_PATH);
    strcat (fullpath, filename);
    if (check_file (fullpath)) return 1;

    // Then check at ~/.voxvision
    strcpy (fullpath, getenv("HOME"));
    strcat (fullpath, "/.voxvision/");
    strcat (fullpath, filename);
    if (check_file (fullpath)) return 1;

    // Then as last resort try environment variable VOXVISION_DATA
    strcpy (fullpath, getenv("VOXVISION_DATA"));
    strcat (fullpath, "/");
    strcat (fullpath, filename);
    if (check_file (fullpath)) return 1;

    return 0;
}
