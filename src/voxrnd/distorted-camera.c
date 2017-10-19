#include <math.h>
#include <assert.h>
#include "simple-camera.h"
#include "distorted-camera.h"
#include "vect-ops.h"

static struct vox_camera* distorted_construct_camera (const struct vox_camera *cam);

static void distorted_screen2world (const struct vox_camera *cam, vox_dot ray, int sx, int sy)
{
    const struct vox_simple_camera *camera = (void*)cam;
    float mul = camera->mul;
    float xsub = camera->xsub;
    float ysub = camera->ysub;

    assert (mul != 0 && xsub != 0 && ysub != 0);
    float psi = mul*sx - xsub;
    float phi = mul*sy - ysub;

    float x1 = cos(phi)*sin(psi);
    float x2 = cos(phi)*cos(psi);
    float x3 = sin(phi);
#ifdef SSE_INTRIN
    _mm_store_ps (ray, _mm_set_ps (0, x3, x2, x1));
#else
    ray[0] = x1;
    ray[1] = x2;
    ray[2] = x3;
#endif

    vox_rotate_vector (camera->rotation, ray, ray);
}

static struct vox_camera* distorted_construct_camera (const struct vox_camera *cam)
{
    struct vox_camera *camera = vox_simple_camera_iface()->construct_camera (cam);
    vox_use_camera_methods (camera, vox_distorted_camera_iface ());

    return camera;
}

static struct vox_camera_interface vox_distorted_camera_interface =
{
    .screen2world = distorted_screen2world,
    .construct_camera = distorted_construct_camera
};

struct vox_camera_interface* vox_distorted_camera_iface ()
{
    return &vox_distorted_camera_interface;
}
