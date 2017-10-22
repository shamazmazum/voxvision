#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "../voxtrees/geom.h"
#include "vect-ops.h"
#include "camera.h"

struct vox_simple_camera
{
    struct vox_camera_interface *iface; /**< \brief Simple camera methods **/
    float xsub, ysub;
    vox_dot position;
    vox_quat rotation;
    float mul, fov;
};

struct vox_camera_interface* get_methods ();

static void simple_screen2world (const struct vox_camera *cam, vox_dot ray, int sx, int sy)
{
    const struct vox_simple_camera *camera = (void*)cam;
    float mul = camera->mul;
    float xsub = camera->xsub;
    float ysub = camera->ysub;

    assert (mul != 0 && xsub != 0 && ysub != 0);
#ifdef SSE_INTRIN
    _mm_store_ps (ray, _mm_set_ps (0, mul*sy - ysub,
                                   1, mul*sx - xsub));
#else
    ray[0] = mul*sx - xsub;
    ray[1] = 1.0;
    ray[2] = mul*sy - ysub;
#endif

    vox_rotate_vector (camera->rotation, ray, ray);
}

static void simple_get_position (const struct vox_camera *cam, vox_dot res)
{
    const struct vox_simple_camera *camera = (void*)cam;
    vox_dot_copy (res, camera->position);
}

static void simple_set_position (struct vox_camera *cam, const vox_dot pos)
{
    struct vox_simple_camera *camera = (void*)cam;
    vox_dot_copy (camera->position, pos);
}

static void simple_set_rot_angles (struct vox_camera *cam, const vox_dot angles)
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

static void simple_move_camera (struct vox_camera *cam, const vox_dot delta)
{
    vox_dot deltatr;
    struct vox_simple_camera *camera = (void*)cam;

    vox_rotate_vector (camera->rotation, delta, deltatr);
    vox_dot_add (camera->position, deltatr, camera->position);
}

static void simple_rotate_camera (struct vox_camera *cam, const vox_dot delta)
{
    struct vox_simple_camera *camera = (void*)cam;
    // Basis unit vector in the camera's coordinate system
    static const vox_dot orts[] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };

    int i,j;
    vox_quat tmp;
    vox_dot ort_rotated;
    /*
      From axis Z to axis X throug axis Y.
      Rotating in that order allows simple_look_at() reuse this method.
    */
    for (i=2; i>=0; i--)
    {
        /*
          After rotation ort[i] is the same vector but in the world coordinate
          system.
        */
        vox_rotate_vector (camera->rotation, orts[i], ort_rotated);
        float sinang = sinf(delta[i]);
        float cosang = cosf(delta[i]);
        for (j=0; j<3; j++)
        {
            tmp[j+1] = sinang*ort_rotated[j];
        }
        tmp[0] = cosang;
        /*
          Now, tmp is a rotation around camera's i-th axis in the world
          coordinate system. Apply this rotation by multiplying tmp and
          camera->rotation quaternions.
        */
        vox_quat_mul (tmp, camera->rotation, camera->rotation);
    }

    /*
     * KLUDGE: Normalize resulting quaternion.
     * Ordinary, we multiply rotation quaternions hence norm of the result is
     * always 1. But there is a cummulative computational error because I use
     * previous rotation to calculate the next. In order to fix it, I must
     * normalize.
     */
    vox_quat_normalize (camera->rotation);
}

static void simple_look_at (struct vox_camera *cam, const vox_dot coord)
{
    struct vox_simple_camera *camera = (void*)cam;

    vox_dot sub, rot;
    vox_dot_sub (camera->position, coord, sub);

    // Reset rotation
    vox_quat_set_identity (camera->rotation);

    rot[0] = -atan2f (sub[2], -sqrtf (sub[0]*sub[0] + sub[1]*sub[1]))/2;
    rot[1] = 0;
    rot[2] = -atan2f (sub[0], sub[1])/2;

    cam->iface->rotate_camera (cam, rot);
}

static void simple_set_window_size (struct vox_camera *cam, int w, int h)
{
    struct vox_simple_camera *camera = (void*)cam;

    if (w > h)
    {
        camera->xsub = camera->fov;
        camera->ysub = camera->fov*h/w;
        camera->mul  = camera->fov*2/w;
    }
    else
    {
        camera->ysub = camera->fov;
        camera->xsub = camera->fov*w/h;
        camera->mul  = camera->fov*2/h;
    }
}

static struct vox_camera* simple_construct_camera (const struct vox_camera *cam)
{
    struct vox_simple_camera *camera;
    const struct vox_simple_camera *old_camera = (void*)cam;

    camera = aligned_alloc (16, sizeof (struct vox_simple_camera));

    if (old_camera != NULL)
        memcpy (camera, old_camera, sizeof (struct vox_simple_camera));
    else
    {
        memset (camera, 0, sizeof (struct vox_simple_camera));
        vox_init_camera ((struct vox_camera*)camera);
        camera->fov = 1.0;
        vox_quat_set_identity (camera->rotation);
    }

    vox_use_camera_methods ((struct vox_camera*)camera, get_methods ());
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
    .look_at = simple_look_at,
    .set_window_size = simple_set_window_size,
    .construct_camera = simple_construct_camera,
    .get_fov = simple_get_fov,
    .set_fov = simple_set_fov
};

struct vox_camera_interface* get_methods ()
{
    return &vox_simple_camera_interface;
}
