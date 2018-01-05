#include "lights.h"

// Ignore color for now
int vox_insert_point_light (struct vox_rnd_ctx *context,
                            const struct vox_sphere *position,
                            Uint32 color)
{
    return vox_mtree_add_sphere (&(context->point_lights), position);
}

int vox_delete_point_light (struct vox_rnd_ctx *context,
                            const struct vox_sphere *position)
{
    return vox_mtree_remove_sphere (&(context->point_lights), position);
}
