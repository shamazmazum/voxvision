#include <math.h>
#include <assert.h>
#include "simple-camera.h"
#include "distorted-camera.h"
#include "vect-ops.h"

static struct vox_camera* distorted_construct_camera (struct vox_camera *cam, ...);
struct vox_camera* distorted_vconstruct_camera (struct vox_camera *cam, va_list args);

static void distorted_screen2world (struct vox_camera *cam, vox_dot ray, int sx, int sy)
{
    struct vox_simple_camera *camera = (void*)cam;
    float xmul = camera->xmul;
    float ymul = camera->ymul;

    assert (xmul != 0 && ymul != 0);
    float psi = xmul*sx - camera->fov;
    float phi = ymul*sy - camera->fov;

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

static struct vox_camera* distorted_construct_camera (struct vox_camera *cam, ...)
{
    va_list args;
    struct vox_camera *camera;

    va_start (args, cam);
    camera = distorted_vconstruct_camera (NULL, args);
    va_end (args);
    return camera;
}

struct vox_camera* distorted_vconstruct_camera (struct vox_camera *cam, va_list args)
{
    struct vox_camera *camera = vox_simple_camera_get_iface()->vconstruct_camera (cam, args);
    camera->iface->vconstruct_camera = distorted_vconstruct_camera;
    camera->iface->construct_camera = distorted_construct_camera;
    camera->iface->screen2world = distorted_screen2world;

    return camera;
}

static struct vox_camera_interface vox_distorted_camera_iface =
{
    .screen2world = distorted_screen2world,
    .vconstruct_camera = distorted_vconstruct_camera,
    .construct_camera = distorted_construct_camera
};

struct vox_camera_interface* vox_distorted_camera_get_iface ()
{
    return &vox_distorted_camera_iface;
}
