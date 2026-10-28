#include "screenshot.h"
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

static bool pick_name (char *name, const char *dirname) {
    static int i = 0;
    int res;
    struct stat sb;

    do {
        sprintf (name, "%s/screenshot%i.bmp", dirname, i);
        res = stat (name, &sb);
        i++;
    } while (res == 0);

    return errno == ENOENT;
}

bool vox_screenshot (const struct vox_rnd_ctx *context, const char *dirname) {
    char name[MAXPATHLEN];
    const char *voxvision_data = getenv ("VOXVISION_DATA");

    bool res = pick_name (name, (dirname != NULL) ? dirname :
                          (voxvision_data != NULL) ? voxvision_data : "");
    if (res == false) {
        return false;
    }

    return SDL_SaveBMP (context->surface, name) == 0;
}
