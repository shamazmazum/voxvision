#include <strings.h>
#include <stdlib.h>
#include "cd.h"
#include "../voxtrees/search.h"

struct vox_cd {
    struct vox_camera *camera;
    struct vox_rnd_ctx *ctx;
    vox_dot last_position;
    float camera_radius;
};

struct vox_cd* vox_make_cd ()
{
    struct vox_cd *cd = aligned_alloc (16, sizeof (struct vox_cd));
    memset (cd, 0, sizeof (struct vox_cd));

    return cd;
}

void vox_cd_attach_camera (struct vox_cd *cd, struct vox_camera *camera, float radius)
{
    cd->camera = camera;
    cd->camera_radius = radius;
    camera->iface->get_position (camera, cd->last_position);
}

void vox_cd_attach_context (struct vox_cd *cd, struct vox_rnd_ctx *ctx)
{
    cd->ctx = ctx;
}

void vox_cd_gravity (struct vox_cd *cd, const vox_dot gravity)
{
    // This function is a stub
}

void vox_cd_collide (struct vox_cd *cd)
{
    struct vox_camera *camera = cd->camera;
    struct vox_rnd_ctx *ctx = cd->ctx;

    if (camera != NULL && ctx != NULL)
    {
        vox_dot position;
        camera->iface->get_position (camera, position);
        if (!vox_dot_equalp (position, cd->last_position))
        {
            if (vox_tree_ball_collidep (ctx->scene, position, cd->camera_radius))
                camera->iface->set_position (camera, cd->last_position);
            else
                vox_dot_copy (cd->last_position, position);
        }
    }
}
