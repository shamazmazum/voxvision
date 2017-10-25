#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <stdio.h>
#include "camera.h"

void vox_use_camera_methods (struct vox_camera *camera, const struct vox_camera_interface *iface)
{
    size_t nfields = sizeof (struct vox_camera_interface)/sizeof(void*);
    size_t i;
    void* const * src = (void * const *)iface;
    void **dst = (void**)camera->iface;

    for (i=0; i<nfields; i++)
    {
        if (src[i] != NULL) dst[i] = src[i];
    }
}

static void dummy_vector (const struct vox_camera *camera, vox_dot ray, ...)
{
    vox_dot_set (ray, 0, 0, 0);
}

static void dummy_void (struct vox_camera *camera, ...)
{
}

static float dummy_fl (struct vox_camera *camera, ...)
{
    return 0;
}

static void* dummy_ptr (struct vox_camera *camera, ...)
{
    return NULL;
}

static void destroy_camera (struct vox_camera *camera)
{
    free (camera->iface);
    free (camera);
}

struct vox_camera_interface dummy_methods = {
    .screen2world = (void*)dummy_vector,
    .rotate_camera = (void*)dummy_void,
    .move_camera = (void*)dummy_void,
    .look_at = (void*)dummy_void,
    .set_rot_angles = (void*)dummy_void,
    .get_position = (void*)dummy_vector,
    .set_position = (void*)dummy_void,
    .get_fov = (void*)dummy_fl,
    .set_fov = (void*)dummy_void,
    .set_window_size = (void*)dummy_void,

    .construct_camera = (void*)dummy_ptr,
    .destroy_camera = destroy_camera
};

void vox_init_camera (struct vox_camera *camera)
{
    camera->iface = malloc (sizeof (struct vox_camera_interface));
    memcpy (camera->iface, &dummy_methods, sizeof (struct vox_camera_interface));
}

/* Some of these are copied from voxtrees library */
static int check_file (const char* filename)
{
    struct stat sb;

    if (!stat (filename, &sb) && S_ISREG(sb.st_mode)) return 1;
    return 0;
}

static int find_module (const char *name, char *fullpath)
{
    // Then as last resort try environment variable VOXVISION_MODULES
    if (snprintf (fullpath, MAXPATHLEN,
                  "%s/%s.so", getenv ("VOXVISION_MODULES"), name) >= MAXPATHLEN)
        return 0;
    if (check_file (fullpath)) return 1;

    // At first, try to find data file in system-wide data directory.
    if (snprintf (fullpath, MAXPATHLEN,
                  "%s%s.so", VOX_MODULE_PATH, name) >= MAXPATHLEN)
        return 0;
    if (check_file (fullpath)) return 1;

    // Then check at ~/.voxvision
    if (snprintf (fullpath, MAXPATHLEN,
                  "%s/.voxvision/%s.so", getenv ("HOME"), name) >= MAXPATHLEN)
        return 0;
    if (check_file (fullpath)) return 1;

    return 0;
}

struct vox_camera_interface* vox_camera_methods (const char *name)
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

    struct vox_camera_interface* (*interface_getter)() =
        (struct vox_camera_interface* (*)()) dlfunc (handle, "get_methods");
    if (interface_getter == NULL) {
        dlclose (handle);
        return NULL;
    }

    struct vox_camera_interface *iface = interface_getter();
    dlclose (handle);

    return iface;
}
