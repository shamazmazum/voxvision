#include <math.h>
#include <stdlib.h>
#include "../voxtrees/search.h"
#include "../voxtrees/geom.h"
#include "vect-ops.h"
#include "camera.h"
#include "renderer.h"

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

static int simple_set_position (void *obj, vox_dot pos)
{
    vox_simple_camera *camera = obj;
    int res = vox_tree_ball_collidep (camera->iface.ctx->scene, pos, camera->body_radius);
    if (res == 0) vox_dot_copy (camera->position, pos);
    return res;
}

static void simple_set_rot_angles (void *obj, float rotx, float roty, float rotz)
{
    vox_simple_camera *camera = obj;

    float sinphi = sinf(rotx);
    float cosphi = cosf(rotx);

    float sinpsi = sinf(roty);
    float cospsi = cosf(roty);

    vox_quat qrotx = {sinphi, 0, 0, cosphi}; // First rotation
    vox_quat qroty = {0, sinpsi, 0, cospsi}; // Second rotation
    vox_quat_mul (qroty, qrotx, camera->rotation);
}

static int simple_move_camera (void *obj, vox_dot delta)
{
    vox_simple_camera *camera = obj;
    vox_rotate_vector (camera->rotation, delta, delta);
    vox_dot new_pos;
    sum_vector (camera->position, delta, new_pos);

    int res = vox_tree_ball_collidep (camera->iface.ctx->scene, new_pos, camera->body_radius);
    if (res == 0) vox_dot_copy (camera->position, new_pos);
    return res;
}

static void simple_rotate_camera (void *obj, vox_dot delta)
{
    vox_simple_camera *camera = obj;

    int m;
    vox_dot i = {1,0,0};
    vox_dot j = {0,1,0};
    vox_dot k = {0,0,1};

    vox_rotate_vector (camera->rotation, i, i);
    vox_rotate_vector (camera->rotation, k, k);

    float sinphi = sinf(delta[0]);
    float cosphi = cosf(delta[0]);

    float sinpsi = sinf(delta[1]);
    float cospsi = cosf(delta[1]);

    vox_quat qi;
    for (m=0; m<3; m++) qi[m] = i[m]*sinphi;
    qi[3] = cosphi;

    vox_quat qk;
    for (m=0; m<3; m++) qk[m] = k[m]*sinpsi;
    qk[3] = cospsi;

    vox_quat tmp;
    vox_quat_mul (qi, camera->rotation, tmp);
    vox_quat_mul (qk, tmp, camera->rotation);
}

vox_simple_camera* vox_make_simple_camera (float fov, vox_dot position)
{
    vox_simple_camera *camera;
    camera = aligned_alloc (16, sizeof (vox_simple_camera));
    vox_dot_copy (camera->position, position);
    camera->fov = fov;
    camera->body_radius = 50;
    memset (camera->rotation, 0, 3*sizeof(float));
    camera->rotation[3] = 1;

    camera->iface.screen2world = simple_screen2world;
    camera->iface.get_position = simple_get_position;
    camera->iface.set_position = simple_set_position;
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
