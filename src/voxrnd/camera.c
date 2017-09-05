#include <string.h>
#include "camera.h"

void inherit_interface (struct vox_camera *camera, const struct vox_camera_interface *iface)
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
