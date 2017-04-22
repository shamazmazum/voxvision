#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "../voxtrees/search.h"
#include "../voxtrees/geom.h"
#include "vect-ops.h"
#include "simple-camera.h"
#include "renderer.h"

static void simple_screen2world (const struct vox_camera *cam, vox_dot ray, int sx, int sy)
{
    const struct vox_simple_camera *camera = (void*)cam;
    float xmul = camera->xmul;
    float ymul = camera->ymul;

    assert (xmul != 0 && ymul != 0);
#ifdef SSE_INTRIN
    _mm_store_ps (ray, _mm_set_ps (0, camera->ymul*sy - camera->fov,
                                   1, camera->xmul*sx - camera->fov));
#else
    ray[0] = camera->xmul*sx - camera->fov;
    ray[1] = 1.0;
    ray[2] = camera->ymul*sy - camera->fov;
#endif

    vox_rotate_vector (camera->rotation, ray, ray);
}

static void simple_get_position (const struct vox_camera *cam, vox_dot res)
{
    const struct vox_simple_camera *camera = (void*)cam;
    vox_dot_copy (res, camera->position);
}

static int simple_set_position (struct vox_camera *cam, vox_dot pos)
{
    struct vox_simple_camera *camera = (void*)cam;
    int res = 0;
    if (camera->ctx != NULL)
        res = vox_tree_ball_collidep (camera->ctx->scene, pos, camera->body_radius);
    if (res == 0) vox_dot_copy (camera->position, pos);
    return res;
}

static void simple_set_rot_angles (struct vox_camera *cam, vox_dot angles)
{
    struct vox_simple_camera *camera = (void*)cam;

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

static int simple_move_camera (struct vox_camera *cam, vox_dot delta)
{
    struct vox_simple_camera *camera = (void*)cam;
    vox_rotate_vector (camera->rotation, delta, delta);
    vox_dot new_pos;
    vox_dot_add (camera->position, delta, new_pos);

    int res = 0;
    if (camera->ctx != NULL)
        res = vox_tree_ball_collidep (camera->ctx->scene, new_pos, camera->body_radius);
    if (res == 0) vox_dot_copy (camera->position, new_pos);
    return res;
}

static void simple_rotate_camera (struct vox_camera *cam, vox_dot delta)
{
    struct vox_simple_camera *camera = (void*)cam;

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

static void simple_set_window_size (struct vox_camera *cam, int w, int h)
{
    struct vox_simple_camera *camera = (void*)cam;

    camera->xmul = 2*camera->fov/w;
    camera->ymul = 2*camera->fov/h;
}

static void simple_destroy_camera (struct vox_camera *cam)
{
    struct vox_simple_camera *camera = (void*)cam;

    free (camera->iface);
    free (camera);
}

static void simple_coerce_class (struct vox_camera *cam)
{
    memcpy (cam->iface, vox_simple_camera_iface(), sizeof (struct vox_camera_interface));
}

static struct vox_camera* simple_construct_camera (const struct vox_camera *cam)
{
    struct vox_simple_camera *camera;
    const struct vox_simple_camera *old_camera = (void*)cam;
    struct vox_camera_interface *iface;

    camera = aligned_alloc (16, sizeof (struct vox_simple_camera));
    iface = malloc (sizeof (struct vox_camera_interface));
    camera->iface = iface;

    if (old_camera != NULL)
    {
        camera->ctx = old_camera->ctx;
        vox_dot_copy (camera->position, old_camera->position);
        camera->fov = old_camera->fov;
        camera->body_radius = old_camera->body_radius;
        vox_quat_copy (camera->rotation, old_camera->rotation);
        camera->xmul = old_camera->xmul;
        camera->ymul = old_camera->ymul;
    }
    else
    {
        camera->ctx = NULL;
        memset (camera->position, 0, sizeof (vox_dot));
        camera->fov = 1.0;
        camera->body_radius = 50;
        memset (camera->rotation, 0, 3*sizeof(float));
        camera->rotation[0] = 1;
        camera->xmul = 0; camera->ymul = 0;
    }

    vox_simple_camera_iface()->coerce_class ((struct vox_camera*)camera);
    return (struct vox_camera*)camera;
}

static void simple_set_fov (struct vox_camera *cam, float fov)
{
    struct vox_simple_camera *camera = (void*)cam;

    camera->fov = fov;
}

static float simple_get_fov (const struct vox_camera *cam)
{
    const struct vox_simple_camera *camera = (void*)cam;

    return camera->fov;
}

static struct vox_camera_interface vox_simple_camera_interface =
{
    .screen2world = simple_screen2world,
    .get_position = simple_get_position,
    .set_position = simple_set_position,
    .set_rot_angles = simple_set_rot_angles,
    .move_camera = simple_move_camera,
    .rotate_camera = simple_rotate_camera,
    .set_window_size = simple_set_window_size,
    .construct_camera = simple_construct_camera,
    .destroy_camera = simple_destroy_camera,
    .coerce_class = simple_coerce_class,
    .get_fov = simple_get_fov,
    .set_fov = simple_set_fov
};

struct vox_camera_interface* vox_simple_camera_iface ()
{
    return &vox_simple_camera_interface;
}

float vox_simple_camera_set_radius (struct vox_simple_camera *camera, float radius)
{
    radius = (radius > 0) ? radius : 0;
    camera->body_radius = radius;
    return radius;
}

float vox_simple_camera_get_radius (struct vox_simple_camera *camera)
{
    return camera->body_radius;
}
