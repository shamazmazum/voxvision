#include <math.h>
#include <assert.h>
#include "simple-camera.h"
#include "distorted-camera.h"
#include "vect-ops.h"

static struct vox_camera* distorted_construct_camera (void *obj, ...);
struct vox_camera* distorted_vconstruct_camera (void *obj, va_list args);

static void distorted_screen2world (void *obj, vox_dot ray, int sx, int sy)
{
    struct vox_simple_camera *camera = obj;
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

static struct vox_camera* distorted_construct_camera (void *obj, ...)
{
    va_list args;
    struct vox_camera *camera;

    va_start (args, obj);
    camera = distorted_vconstruct_camera (NULL, args);
    va_end (args);
    return camera;
}

struct vox_camera* distorted_vconstruct_camera (void *obj, va_list args)
{
    struct vox_camera *camera = vox_simple_camera_interface.vconstruct_camera (obj, args);
    camera->iface->vconstruct_camera = distorted_vconstruct_camera;
    camera->iface->construct_camera = distorted_construct_camera;
    camera->iface->screen2world = distorted_screen2world;

    return camera;
}

struct vox_camera_interface vox_distorted_camera_interface =
{
    .screen2world = distorted_screen2world,
    .vconstruct_camera = distorted_vconstruct_camera,
    .construct_camera = distorted_construct_camera
};
