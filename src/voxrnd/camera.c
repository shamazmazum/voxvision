#include <math.h>
#include <stdlib.h>
#include "../voxtrees/search.h"
#include "../voxtrees/geom.h"
#include "vect-ops.h"
#include "camera.h"
#include "renderer.h"

static void simple_update_rotation (vox_simple_camera *camera)
{
    // Update rotation quaternion
    float sinphi = sinf(camera->rotx);
    float cosphi = cosf(camera->rotx);

    float sinpsi = sinf(camera->roty);
    float cospsi = cosf(camera->roty);

#if 0
    vox_quat rotx = {sinphi, 0, 0, cosphi}; // First rotation
    vox_quat roty = {0, sinpsi, 0, cospsi}; // Second rotation
    vox_quat_mul (roty, rotx, camera->rotation);
#else
    camera->rotation[0] = sinphi*cospsi;
    camera->rotation[1] = cosphi*sinpsi;
    camera->rotation[2] = -sinphi*sinpsi;
    camera->rotation[3] = cosphi*cospsi;
#endif
}

static void simple_screen2world (void *obj, vox_dot ray, int sx, int sy)
{
    vox_simple_camera *camera = obj;
    SDL_Surface *surface = camera->iface.ctx->surface;
    ray[0] = camera->fov*(2.0*sx/surface->w - 1.0);
    ray[1] = 1.0;
    ray[2] = camera->fov*(2.0*sy/surface->h - 1.0);

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

static void simple_move_camera (void *obj, vox_dot delta)
{
    vox_simple_camera *camera = obj;
    vox_rotate_vector (camera->rotation, delta, delta);
    vox_dot new_pos;
    sum_vector (camera->position, delta, new_pos);

    if (!(vox_tree_ball_collidep (camera->iface.ctx->scene, new_pos, camera->body_radius)))
        vox_dot_copy (camera->position, new_pos);
}

static void simple_rotate_camera (void *obj, vox_dot delta)
{
    vox_simple_camera *camera = obj;
    camera->rotx += delta[0];
    camera->roty += delta[1];
    camera->rotz += delta[2];
    simple_update_rotation (camera);
}

vox_simple_camera* vox_make_simple_camera (float fov, vox_dot position)
{
    vox_simple_camera *camera;
    camera = aligned_alloc (16, sizeof (vox_simple_camera));
    vox_dot_copy (camera->position, position);
    camera->fov = fov;
    camera->body_radius = 50;
    simple_set_rot_angles (camera, 0, 0, 0);

    camera->iface.screen2world = simple_screen2world;
    camera->iface.get_position = simple_get_position;
    camera->iface.get_rot_angles = simple_get_rot_angles;
    camera->iface.set_rot_angles = simple_set_rot_angles;
    camera->iface.move_camera = simple_move_camera;
    camera->iface.rotate_camera = simple_rotate_camera;
    camera->iface.camera = camera;
    return camera;
}

void vox_simple_camera_set_radius (vox_simple_camera *camera, float radius)
{
    camera->body_radius = radius;
}
