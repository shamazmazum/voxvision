#include <string.h>
#include "camera.h"
#include "modules.h"

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

static void destroy_camera (struct vox_camera *camera)
{
    free (camera->iface);
    free (camera);
}

struct vox_camera_interface dummy_methods = {
    .screen2world = (void*)vox_method_dot_dummy,
    .rotate_camera = (void*)vox_method_void_dummy,
    .move_camera = (void*)vox_method_void_dummy,
    .look_at = (void*)vox_method_void_dummy,
    .get_position = (void*)vox_method_dot_dummy,
    .set_window_size = (void*)vox_method_void_dummy,

    .destroy_camera = destroy_camera,
    VOX_OBJECT_METHODS
};

void vox_init_camera (struct vox_camera *camera)
{
    camera->iface = malloc (sizeof (struct vox_camera_interface));
    memcpy (camera->iface, &dummy_methods, sizeof (struct vox_camera_interface));
}

struct vox_camera_interface* vox_camera_methods (const char *name)
{
    struct vox_camera_interface* iface =
        (struct vox_camera_interface*)vox_load_module (name, CAMERA_MODULE);
    return iface;
}
