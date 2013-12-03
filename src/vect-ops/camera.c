#include <math.h>
#include "vect-ops.h"
#include "camera.h"

#define RAY_DIST 400.0

void update_camera_data (vox_camera *camera)
{
    // Update dx and dy
    //aux_ctx->dx = 2.0*RAY_DIST*camera->fov/aux_ctx->surface->w;
    //aux_ctx->dy = 2.0*RAY_DIST*camera->fov/aux_ctx->surface->h;

    // Update rotation quaternion
    float sinphi = sinf(camera->phi);
    float cosphi = cosf(camera->phi);

    float sinpsi = sinf(camera->psi);
    float cospsi = cosf(camera->psi);
    camera->rotation[0] = sinphi*cospsi;
    camera->rotation[1] = -sinphi*sinpsi;
    camera->rotation[2] = cosphi*sinpsi;
    camera->rotation[3] = cosphi*cospsi;
}

void screen_2_world (vox_camera *camera, vox_dot ray, int w, int h, int sx, int sy)
{
    ray[0] = RAY_DIST*camera->fov*(2.0*sx/w - 1.0);
    ray[1] = RAY_DIST;
    ray[2] = RAY_DIST*camera->fov*(2.0*sy/h - 1.0);

    vox_rotate_vector (camera->rotation, ray, ray);
}
