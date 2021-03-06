#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "camera.h"
#include "modules.h"
#include "../voxtrees/geom.h"

struct vox_doom_camera
{
    struct vox_camera_interface *iface;
    float xsub, ysub;
    vox_dot position;
    float phi, sinphi, cosphi, k;
    float mul, fov;
};

struct vox_module* module_init();

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

    vox_dot_set (res, xtr, ytr, ztr);
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
    dir[2] = ysub - mul*sy;
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

static void doom_set_property_number (struct vox_camera *cam, const char *name, float value)
{
    struct vox_doom_camera *camera = (struct vox_doom_camera*)cam;
    if (strcmp (name, "fov") == 0) camera->fov = value;
}

static void doom_set_property_dot (struct vox_camera *cam, const char *name, const vox_dot value)
{
    struct vox_doom_camera *camera = (struct vox_doom_camera*)cam;
    if (strcmp (name, "position") == 0) {
        vox_dot_copy (camera->position, value);
    } else if (strcmp (name, "rotation") == 0) {
        camera->k = value[0];
        camera->phi = value[2];
        camera->sinphi = sinf (camera->phi);
        camera->cosphi = cosf (camera->phi);
    }
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

static void doom_look_at (struct vox_camera *cam, const vox_dot coord)
{
    struct vox_doom_camera *camera = (void*)cam;

    vox_dot sub;
    vox_dot_sub (coord, camera->position, sub);

    camera->k = sub[2] / hypotf (sub[0], sub[1]);
    camera->phi = -atan2f (sub[0], sub[1]);
    camera->cosphi = cosf (camera->phi);
    camera->sinphi = sinf (camera->phi);
}

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

    camera = vox_alloc (sizeof (struct vox_doom_camera));

    if (old_camera != NULL)
        memcpy (camera, old_camera, sizeof (struct vox_doom_camera));
    else
    {
        memset (camera, 0, sizeof (struct vox_doom_camera));
        vox_init_camera ((struct vox_camera*)camera);
        camera->fov = 1.0;
        camera->cosphi = 1.0;
    }

    vox_use_camera_methods ((struct vox_camera*)camera,
                            (struct vox_camera_interface*)module_init()->methods);
    return (struct vox_camera*)camera;
}

static struct vox_camera_interface vox_doom_camera_interface =
{
    .screen2world = doom_screen2world,
    .get_position = doom_get_position,
    .move_camera = doom_move_camera,
    .rotate_camera = doom_rotate_camera,
    .look_at = doom_look_at,
    .set_window_size = doom_set_window_size,
    .construct_camera = doom_construct_camera,
    .set_property_number = doom_set_property_number,
    .set_property_dot = doom_set_property_dot
};

static struct vox_module vox_doom_camera_module =
{
    .type = CAMERA_MODULE,
    .methods = (struct vox_module_methods*)&vox_doom_camera_interface
};

struct vox_module* module_init ()
{
    return &vox_doom_camera_module;
}
