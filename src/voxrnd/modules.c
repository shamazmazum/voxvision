#include <sys/param.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "modules.h"
#include "../voxvision.h"

/* Some of these are copied from voxtrees library */
static int check_file (const char* filename)
{
    struct stat sb;

    if (!stat (filename, &sb) && S_ISREG(sb.st_mode)) return 1;
    return 0;
}

static int find_module (const char *name, char *fullpath)
{
    /* At first, try to find data file in system-wide data directory. */
    if (snprintf (fullpath, MAXPATHLEN,
                  "%s%s.so", VOX_MODULE_PATH, name) >= MAXPATHLEN)
        return 0;
    if (check_file (fullpath)) return 1;

    /*
     * Then try at location specified by VOXVISION_MODULES environment variable.
     * voxtrees library defaults VOXVISION_MODULES to ~/.voxvision directory.
     */
    if (snprintf (fullpath, MAXPATHLEN,
                  "%s/%s.so", getenv ("VOXVISION_MODULES"), name) >= MAXPATHLEN)
        return 0;
    if (check_file (fullpath)) return 1;

    return 0;
}

struct vox_module_methods* vox_load_module (const char *name, int type)
{
    char fullpath[MAXPATHLEN];
    int found = find_module (name, fullpath);
    if (!found) return NULL;

    /*
     * Instead of track references to modules and store pointers to methods in
     * the hash table, we open modules with RTLD_NODELETE, so closure only
     * decrements reference counter and leaves a pointer to methods valid.
     */
    void *handle = dlopen (fullpath, RTLD_LAZY | RTLD_NODELETE);
    if (handle == NULL) return NULL;

    struct vox_module* (*module_init)() =
        (struct vox_module* (*)()) dlfunc (handle, "module_init");
    if (module_init == NULL) {
        dlclose (handle);
        return NULL;
    }
    dlclose (handle);

    struct vox_module *module = module_init();
    if (module->type != type) return NULL;

    return module->methods;
}
