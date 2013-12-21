#include <math.h>
#include <assert.h>
#include "vect-ops.h"
#include "camera.h"

#define RAY_DIST 400.0

static void simple_update_rotation (vox_simple_camera *camera)
{
    // Update dx and dy
    //aux_ctx->dx = 2.0*RAY_DIST*camera->fov/aux_ctx->surface->w;
    //aux_ctx->dy = 2.0*RAY_DIST*camera->fov/aux_ctx->surface->h;

    // Update rotation quaternion
    float sinphi = sinf(camera->phi);
    float cosphi = cosf(camera->phi);

    float sinpsi = sinf(camera->psi);
    float cospsi = cosf(camera->psi);
    camera->rotation[0] = sinphi*cospsi;
    camera->rotation[1] = sinphi*sinpsi;
    camera->rotation[2] = cosphi*sinpsi;
    camera->rotation[3] = cosphi*cospsi;
}

void vox_make_simple_camera (vox_simple_camera *camera, float fov, vox_dot position)
{
    camera->obj_type = VOX_CAMERA_SIMPLE;
    vox_dot_copy (camera->position, position);
    camera->fov = fov;

    camera->phi = 0.0;
    camera->psi = 0.0;
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

static DEF_SIMPLE_GETTER_IMPL(vox_simple_camera, phi, float);
static float SETTER_IMPL_NAME(vox_simple_camera, phi) (class_t *obj, float phi)
{
    vox_simple_camera *camera = (vox_simple_camera*)obj;
    camera->phi = phi;
    simple_update_rotation (camera);
    return phi;
}

static DEF_SIMPLE_GETTER_IMPL(vox_simple_camera, psi, float);
static float SETTER_IMPL_NAME(vox_simple_camera, psi) (class_t *obj, float psi)
{
    vox_simple_camera *camera = (vox_simple_camera*)obj;
    camera->psi = psi;
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

    GETTER_DISPATCH (phi, float)
    SETTER_DISPATCH (phi, float)
    
    GETTER_DISPATCH (psi, float)
    SETTER_DISPATCH (psi, float)
} camera_dispatch_table[] =
{{simple_screen2world,
  simple_position_ptr,
  GETTER_IMPL_NAME(vox_simple_camera, fov),
  SETTER_IMPL_NAME(vox_simple_camera, fov),
  
  GETTER_IMPL_NAME(vox_simple_camera, phi),
  SETTER_IMPL_NAME(vox_simple_camera, phi),
  
  GETTER_IMPL_NAME(vox_simple_camera, psi),
  SETTER_IMPL_NAME(vox_simple_camera, psi)}};

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

DEF_SETTER(camera_dispatch_table, psi, float)
DEF_GETTER(camera_dispatch_table, psi, float)

DEF_SETTER(camera_dispatch_table, phi, float)
DEF_GETTER(camera_dispatch_table, phi, float)
