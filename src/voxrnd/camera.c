#include <math.h>
#include <stdio.h>
#include "vect-ops.h"
#include "camera.h"

static void simple_update_rotation (vox_camera *camera)
{
    // Update rotation quaternion
    float sinphi = sinf(camera->rotx);
    float cosphi = cosf(camera->rotx);

    float sinpsi = sinf(camera->rotz);
    float cospsi = cosf(camera->rotz);

#if 0
    vox_quat rotx = {sinphi, 0, 0, cosphi}; // First rotation
    vox_quat rotz = {0, 0, sinpsi, cospsi}; // Second rotation
    vox_quat_mul (rotz, rotx, camera->rotation);
#else
    camera->rotation[0] = sinphi*cospsi;
    camera->rotation[1] = sinphi*sinpsi;
    camera->rotation[2] = cosphi*sinpsi;
    camera->rotation[3] = cosphi*cospsi;
#endif
}

static void simple_screen2world (vox_camera *camera, vox_dot ray, int w, int h, int sx, int sy)
{
    ray[0] = camera->fov*(2.0*sx/w - 1.0);
    ray[1] = 1.0;
    ray[2] = camera->fov*(2.0*sy/h - 1.0);

    vox_rotate_vector (camera->rotation, ray, ray);
}

void vox_make_simple_camera (vox_camera *camera, float fov, vox_dot position)
{
    vox_dot_copy (camera->position, position);
    camera->fov = fov;

    camera->rotx = 0.0;
    camera->rotz = 0.0;
    simple_update_rotation (camera);

    camera->update_rotation = simple_update_rotation;
    camera->screen2world = simple_screen2world;
}
