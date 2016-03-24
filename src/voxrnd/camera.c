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
    float xmul = camera->xmul;
    float ymul = camera->ymul;

    assert (xmul != 0 && ymul != 0);
    ray[0] = camera->xmul*sx - camera->fov;
    ray[1] = 1.0;
    ray[2] = camera->ymul*sy - camera->fov;

    vox_rotate_vector (camera->rotation, ray, ray);
}

static float* simple_get_position (void *camera) {return ((vox_simple_camera*)camera)->position;}

static int simple_set_position (void *obj, vox_dot pos)
{
    vox_simple_camera *camera = obj;
    int res = vox_tree_ball_collidep (camera->iface->ctx->scene, pos, camera->body_radius);
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
        r[i][i+1] = sinf(angles[i]);
        r[i][0] = cosf(angles[i]);
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

    assert (camera->iface->ctx);
    int res = vox_tree_ball_collidep (camera->iface->ctx->scene, new_pos, camera->body_radius);
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
        // Basis unit vectors in the world coordinate system
        ort[i][i] = 1;
        /*
          After rotation it is the basis unit vectors of camera's
          coordinate system.
        */
        vox_rotate_vector (camera->rotation, ort[i], ort[i]);
        float sinang = sinf(delta[i]);
        float cosang = cosf(delta[i]);
        for (j=0; j<3; j++)
        {
            r[i][j+1] = sinang*ort[i][j];
        }
        r[i][0] = cosang;
    }
    /*
      For each axis i, r[i] has a rotation in the world coordinate system
      around camera's basis unit vector i.
      Resulting rotation is a composition of all these rotations
    */

    vox_quat_mul (r[0], camera->rotation, tmp);
    vox_quat_mul (r[1], tmp, r[0]);
    vox_quat_mul (r[2], r[0], camera->rotation);
}

static void simple_set_window_size (void *obj, int w, int h)
{
    vox_simple_camera *camera = obj;

    camera->xmul = 2*camera->fov/w;
    camera->ymul = 2*camera->fov/h;
}

static void simple_destroy_camera (void *obj)
{
    vox_simple_camera *camera = obj;

    free (camera->iface);
    free (camera);
}

vox_simple_camera* vox_make_simple_camera (float fov, vox_dot position)
{
    vox_simple_camera *camera;
    camera = aligned_alloc (16, sizeof (vox_simple_camera));
    struct vox_camera_interface *iface = malloc (sizeof (struct vox_camera_interface));
    camera->iface = iface;
    vox_dot_copy (camera->position, position);
    camera->fov = fov;
    camera->body_radius = 50;
    bzero (camera->rotation, 3*sizeof(float));
    camera->rotation[3] = 1;
    camera->xmul = 0; camera->ymul = 0;

    iface->screen2world = simple_screen2world;
    iface->get_position = simple_get_position;
    iface->set_position = simple_set_position;
    iface->set_rot_angles = simple_set_rot_angles;
    iface->move_camera = simple_move_camera;
    iface->rotate_camera = simple_rotate_camera;
    iface->set_window_size = simple_set_window_size;
    iface->destroy_camera = simple_destroy_camera;
    iface->camera = camera;
    iface->ctx = NULL;
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
