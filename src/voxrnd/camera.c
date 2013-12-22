#include <math.h>
#include <assert.h>
#include <stdio.h>
#include "vect-ops.h"
#include "camera.h"

#define RAY_DIST 400.0

static void simple_update_rotation (vox_simple_camera *camera)
{
    // Update rotation quaternion
    float sinphi = sinf(camera->rotx);
    float cosphi = cosf(camera->rotx);

    float sinpsi = sinf(camera->rotz);
    float cospsi = cosf(camera->rotz);

#if 0
    vox_quat rotx = {sinphi, 0, 0, cosphi}; // First rotation
    vox_quat rotz = {0, 0, sinpsi, cospsi}; // Second rotation
    vox_quat_mul (rotz, rotx, camera->rotation);
#else
    camera->rotation[0] = sinphi*cospsi;
    camera->rotation[1] = sinphi*sinpsi;
    camera->rotation[2] = cosphi*sinpsi;
    camera->rotation[3] = cosphi*cospsi;
#endif
}

void vox_make_simple_camera (vox_simple_camera *camera, float fov, vox_dot position)
{
    camera->obj_type = VOX_CAMERA_SIMPLE;
    vox_dot_copy (camera->position, position);
    camera->fov = fov;

    camera->rotx = 0.0;
    camera->rotz = 0.0;
    simple_update_rotation (camera);
}

static void simple_screen2world (const class_t *cam, vox_dot ray, int w, int h, int sx, int sy)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    ray[0] = RAY_DIST*camera->fov*(2.0*sx/w - 1.0);
    ray[1] = RAY_DIST;
    ray[2] = RAY_DIST*camera->fov*(2.0*sy/h - 1.0);

    vox_rotate_vector (camera->rotation, ray, ray);
}

static DEF_SIMPLE_GETTER_IMPL(vox_simple_camera, fov, float);
static DEF_SIMPLE_SETTER_IMPL(vox_simple_camera, fov, float);

static DEF_SIMPLE_GETTER_IMPL(vox_simple_camera, rotx, float);
static float SETTER_IMPL_NAME(vox_simple_camera, rotx) (class_t *obj, float phi)
{
    vox_simple_camera *camera = (vox_simple_camera*)obj;
    camera->rotx = phi;
    simple_update_rotation (camera);
    return phi;
}

static float GETTER_IMPL_NAME(vox_simple_camera, roty) (const class_t *obj)
{
    fprintf (stderr, "6 degrees of freedom is not implemented in this camera\n");
    return 0;
}
static float SETTER_IMPL_NAME(vox_simple_camera, roty) (class_t *obj, float theta)
{
    fprintf (stderr, "6 degrees of freedom is not implemented in this camera\n");
    return theta;
}

static DEF_SIMPLE_GETTER_IMPL(vox_simple_camera, rotz, float);
static float SETTER_IMPL_NAME(vox_simple_camera, rotz) (class_t *obj, float psi)
{
    vox_simple_camera *camera = (vox_simple_camera*)obj;
    camera->rotz = psi;
    simple_update_rotation (camera);
    return psi;
}
    
static float* simple_position_ptr (const class_t *cam)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    return &(camera->position);
}

static const struct
{
    void   (*screen2world) (const class_t*, vox_dot, int, int, int, int);
    float* (*camera_position_ptr) (const class_t*);
    GETTER_DISPATCH (fov, float)
    SETTER_DISPATCH (fov, float)

    GETTER_DISPATCH (rotx, float)
    SETTER_DISPATCH (rotx, float)
    
    GETTER_DISPATCH (roty, float)
    SETTER_DISPATCH (roty, float)

    GETTER_DISPATCH (rotz, float)
    SETTER_DISPATCH (rotz, float)
} camera_dispatch_table[] =
{{simple_screen2world,
  simple_position_ptr,
  GETTER_IMPL_NAME(vox_simple_camera, fov),
  SETTER_IMPL_NAME(vox_simple_camera, fov),
  
  GETTER_IMPL_NAME(vox_simple_camera, rotx),
  SETTER_IMPL_NAME(vox_simple_camera, rotx),

  GETTER_IMPL_NAME(vox_simple_camera, roty),
  SETTER_IMPL_NAME(vox_simple_camera, roty),

  GETTER_IMPL_NAME(vox_simple_camera, rotz),
  SETTER_IMPL_NAME(vox_simple_camera, rotz)}};

void vox_camera_screen2world (const class_t *cam, vox_dot dir, int w, int h, int sx, int sy)
{
    camera_dispatch_table[cam->obj_type].screen2world (cam, dir, w, h, sx, sy);
}

float* vox_camera_position_ptr (const class_t *cam)
{
    return camera_dispatch_table[cam->obj_type].camera_position_ptr (cam);
}

DEF_SETTER(camera_dispatch_table, fov, float)
DEF_GETTER(camera_dispatch_table, fov, float)

DEF_SETTER(camera_dispatch_table, rotx, float)
DEF_GETTER(camera_dispatch_table, rotx, float)

DEF_SETTER(camera_dispatch_table, roty, float)
DEF_GETTER(camera_dispatch_table, roty, float)

DEF_SETTER(camera_dispatch_table, rotz, float)
DEF_GETTER(camera_dispatch_table, rotz, float)
