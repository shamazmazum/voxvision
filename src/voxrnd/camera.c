#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../voxtrees/geom.h"
#include "vect-ops.h"
#include "camera.h"

static void simple_update_rotation (vox_simple_camera *camera)
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

static void simple_screen2world (void *obj, vox_dot ray, int w, int h, int sx, int sy)
{
    vox_simple_camera *camera = obj;
    ray[0] = camera->fov*(2.0*sx/w - 1.0);
    ray[1] = 1.0;
    ray[2] = camera->fov*(2.0*sy/h - 1.0);

    vox_rotate_vector (camera->rotation, ray, ray);
}

static float* simple_get_position (void *camera) {return ((vox_simple_camera*)camera)->position;}
static void simple_get_rot_angles (void *obj, float *rotx, float *roty, float *rotz)
{
    vox_simple_camera *camera = obj;
    *rotx = camera->rotx;
    *roty = camera->roty;
    *rotz = camera->rotz;
}
static void simple_set_rot_angles (void *obj, float rotx, float roty, float rotz)
{
    vox_simple_camera *camera = obj;
    camera->rotx = rotx;
    camera->roty = roty;
    camera->rotz = rotz;
    simple_update_rotation (camera);
}

static void simple_move_camera (void *obj, vox_dot step, int (*test) (vox_dot, void*), void *param)
{
    vox_simple_camera *camera = obj;
    vox_rotate_vector (camera->rotation, step, step);

    if (test != NULL)
    {
        vox_dot new_pos;
        sum_vector (camera->position, step, new_pos);
        if (test (new_pos, param)) vox_dot_copy (camera->position, new_pos);
    }
    else sum_vector (camera->position, step, camera->position);
}

vox_simple_camera* vox_make_simple_camera (float fov, vox_dot position)
{
    vox_simple_camera *camera;
    void *space;
#ifdef SSE_INTRIN
    space = malloc (sizeof (vox_simple_camera)+16);
    camera = (vox_simple_camera*)(((unsigned long)space+15)&~(unsigned long)15);
#else
    space = malloc (sizeof (vox_simple_camera));
    camera = space;
#endif
    camera->space = space;
    vox_dot_copy (camera->position, position);
    camera->fov = fov;
    simple_set_rot_angles (camera, 0, 0, 0);

    camera->iface.screen2world = simple_screen2world;
    camera->iface.get_position = simple_get_position;
    camera->iface.get_rot_angles = simple_get_rot_angles;
    camera->iface.set_rot_angles = simple_set_rot_angles;
    camera->iface.move_camera = simple_move_camera;
    camera->iface.camera = camera;
    return camera;
}

void vox_destroy_simple_camera (vox_simple_camera *camera)
{
    free (camera->space);
}
