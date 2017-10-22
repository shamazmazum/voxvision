#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "camera.h"
#include "../voxtrees/geom.h"

struct vox_doom_camera
{
    struct vox_camera_interface *iface;
    float xsub, ysub;
    vox_dot position;
    float phi, sinphi, cosphi, k;
    float mul, fov;
};
struct vox_camera_interface* get_methods ();

/*
 * This function performs vector rotation according to the camera's orientation
 * in space. It does not actually rotate a vector and can bring some distortion
 * when looking up and down, as it was in early FPS game (Doom, for example).
 */
static void transform_vector (const struct vox_doom_camera *camera, const vox_dot dot, vox_dot res)
{
    float sinphi = camera->sinphi;
    float cosphi = camera->cosphi;

    float xtr = dot[0]*cosphi - dot[1]*sinphi;
    float ytr = dot[1]*cosphi + dot[0]*sinphi;
    float ztr = dot[2];

#ifdef SSE_INTRIN
    _mm_store_ps (res, _mm_set_ps (0, ztr, ytr, xtr));
#else
    res[0] = xtr;
    res[1] = ytr;
    res[2] = ztr;
#endif
}

static void doom_screen2world (const struct vox_camera *cam, vox_dot ray, int sx, int sy)
{
    const struct vox_doom_camera *camera = (void*)cam;
    float mul = camera->mul;
    float xsub = camera->xsub;
    float ysub = camera->ysub;

    assert (mul != 0 && xsub != 0 && ysub != 0);

    vox_dot dir;
    dir[0] = mul*sx - xsub;
    dir[1] = 1;
    dir[2] = mul*sy - ysub;
    /*
     * Here the norm of the original vector is not saved.
     * To perform fast transformation, we just add a value k to the vector's Z
     * coordinate.
     */
    dir[2] += camera->k;

    transform_vector (camera, dir, ray);
}

static void doom_get_position (const struct vox_camera *cam, vox_dot res)
{
    const struct vox_doom_camera *camera = (void*)cam;
    vox_dot_copy (res, camera->position);
}

static void doom_set_position (struct vox_camera *cam, const vox_dot pos)
{
    struct vox_doom_camera *camera = (void*)cam;
    vox_dot_copy (camera->position, pos);
}

static void doom_set_rot_angles (struct vox_camera *cam, const vox_dot angles)
{
    struct vox_doom_camera *camera = (void*)cam;

    camera->k = angles[0];
    camera->phi = angles[2];
    camera->sinphi = sinf (camera->phi);
    camera->cosphi = cosf (camera->phi);
}

static void doom_move_camera (struct vox_camera *cam, const vox_dot delta)
{
    struct vox_doom_camera *camera = (void*)cam;

    vox_dot deltatr;
    transform_vector (camera, delta, deltatr);    
    vox_dot_add (camera->position, deltatr, camera->position);
}

static void doom_rotate_camera (struct vox_camera *cam, const vox_dot delta)
{
    struct vox_doom_camera *camera = (void*)cam;

    camera->k += delta[0];
    camera->phi += delta[2];
    camera->sinphi = sinf (camera->phi);
    camera->cosphi = cosf (camera->phi);
}

/*
static void simple_look_at (struct vox_camera *cam, vox_dot coord)
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
*/

static void doom_set_window_size (struct vox_camera *cam, int w, int h)
{
    struct vox_doom_camera *camera = (void*)cam;

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

static struct vox_camera* doom_construct_camera (const struct vox_camera *cam)
{
    struct vox_doom_camera *camera;
    const struct vox_doom_camera *old_camera = (void*)cam;

    camera = aligned_alloc (16, sizeof (struct vox_doom_camera));

    if (old_camera != NULL)
        memcpy (camera, old_camera, sizeof (struct vox_doom_camera));
    else
    {
        memset (camera, 0, sizeof (struct vox_doom_camera));
        vox_init_camera ((struct vox_camera*)camera);
        camera->fov = 1.0;
        camera->cosphi = 1.0;
    }

    vox_use_camera_methods ((struct vox_camera*)camera, get_methods ());
    return (struct vox_camera*)camera;
}

static void doom_set_fov (struct vox_camera *cam, float fov)
{
    struct vox_doom_camera *camera = (void*)cam;

    camera->fov = fov;
}

static float doom_get_fov (const struct vox_camera *cam)
{
    const struct vox_doom_camera *camera = (void*)cam;

    return camera->fov;
}

static struct vox_camera_interface vox_doom_camera_interface =
{
    .screen2world = doom_screen2world,
    .get_position = doom_get_position,
    .set_position = doom_set_position,
    .set_rot_angles = doom_set_rot_angles,
    .move_camera = doom_move_camera,
    .rotate_camera = doom_rotate_camera,
//    .look_at = fivex_look_at,
    .set_window_size = doom_set_window_size,
    .construct_camera = doom_construct_camera,
    .get_fov = doom_get_fov,
    .set_fov = doom_set_fov
};

struct vox_camera_interface* get_methods ()
{
    return &vox_doom_camera_interface;
}
