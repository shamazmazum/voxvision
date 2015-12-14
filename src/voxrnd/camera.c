#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "../voxtrees/search.h"
#include "../voxtrees/geom.h"
#include "vect-ops.h"
#include "camera.h"
#include "renderer.h"

static void simple_screen2world (void *obj, vox_dot ray, int sx, int sy)
{
    vox_simple_camera *camera = obj;
    assert (camera->iface.ctx);
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

static void simple_set_rot_angles (void *obj, vox_dot angles)
{
    vox_simple_camera *camera = obj;

    int i;
    vox_quat r[3], tmp;
    memset (r, 0, sizeof(vox_quat)*3);

    for (i=0; i<3; i++)
    {
        r[i][i] = sinf(angles[i]);
        r[i][3] = cosf(angles[i]);
    }

    vox_quat_mul (r[1], r[0], tmp);
    vox_quat_mul (r[2], tmp, camera->rotation);
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

    int i,j;
    vox_dot ort[3];
    vox_quat r[3], tmp;
    memset (ort, 0, sizeof(vox_dot)*3);
    for (i=0; i<3; i++)
    {
        ort[i][i] = 1;
        vox_rotate_vector (camera->rotation, ort[i], ort[i]);
        float sinang = sinf(delta[i]);
        float cosang = cosf(delta[i]);
        for (j=0; j<3; j++)
        {
            r[i][j] = sinang*ort[i][j];
        }
        r[i][3] = cosang;
    }

    vox_quat_mul (r[0], camera->rotation, tmp);
    vox_quat_mul (r[1], tmp, r[0]);
    vox_quat_mul (r[2], r[0], camera->rotation);
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
    camera->iface.ctx = NULL;
    return camera;
}

float vox_simple_camera_set_radius (vox_simple_camera *camera, float radius)
{
    radius = (radius > 0) ? radius : 0;
    camera->body_radius = radius;
    return radius;
}

float vox_simple_camera_get_radius (vox_simple_camera *camera)
{
    return camera->body_radius;
}
