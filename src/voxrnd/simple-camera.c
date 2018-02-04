#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "../voxtrees-ng/geom.h"
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
    vox_dot_set (ray, mul*sx - xsub, 1, mul*sy - ysub);
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
    vox_quat r[3];

    /*
     * NB: We divide angles by 2 because rotation function rotates
     * by doubled angle.
     */
    vox_quat_set (r[0], cosf (angles[0]/2), sinf (angles[0]/2), 0, 0);
    vox_quat_set (r[1], cosf (angles[1]/2), 0, sinf (angles[1]/2), 0);
    vox_quat_set (r[2], cosf (angles[2]/2), 0, 0, sinf (angles[2]/2));

    vox_quat_mul (r[1], r[0], camera->rotation);
    vox_quat_mul (r[2], camera->rotation, camera->rotation);
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
    static const vox_quat orts[] = {
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };

    int i;
    vox_quat ort_rotated;
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
        vox_rotate_vector_q (camera->rotation, orts[i], ort_rotated);
        /*
         * NB: We divide angles by 2 because rotation function rotates
         * by doubled angle.
         */
        float sinang = sinf(delta[i]/2);
        float cosang = cosf(delta[i]/2);

#ifdef SSE_INTRIN
        __v4sf tmp = _mm_set1_ps (sinang) * _mm_load_ps (ort_rotated);
        tmp = _mm_blend_ps (tmp, _mm_set1_ps (cosang), 1);
        _mm_store_ps (ort_rotated, tmp);
#else
        int j;
        for (j=0; j<3; j++)
            ort_rotated[j+1] = sinang*ort_rotated[j+1];
        ort_rotated[0] = cosang;
#endif
        /*
         * Now, ort_rotated is a rotation around camera's i-th axis in the world
         * coordinate system. Apply this rotation by multiplying tmp and
         * camera->rotation quaternions.
        */
        vox_quat_mul (ort_rotated, camera->rotation, camera->rotation);
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
    vox_dot_sub (coord, camera->position, sub);

    // Reset rotation
    vox_quat_set_identity (camera->rotation);
    vox_dot_set (rot, atan2f (sub[2], hypotf (sub[0], sub[1])),
                 0, -atan2f (sub[0], sub[1]));
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
