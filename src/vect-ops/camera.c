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
    camera->rotation[1] = -sinphi*sinpsi;
    camera->rotation[2] = cosphi*sinpsi;
    camera->rotation[3] = cosphi*cospsi;
}

void vox_make_simple_camera (vox_simple_camera *camera, float fov, vox_dot position)
{
    camera->cam_type = VOX_CAMERA_MAX;
    vox_dot_copy (camera->position, position);
    camera->fov = fov;

    camera->phi = 0.0;
    camera->psi = 0.0;
    simple_update_rotation (camera);
}

static void simple_screen_2_world (vox_camera *cam, vox_dot ray, int w, int h, int sx, int sy)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    ray[0] = RAY_DIST*camera->fov*(2.0*sx/w - 1.0);
    ray[1] = RAY_DIST;
    ray[2] = RAY_DIST*camera->fov*(2.0*sy/h - 1.0);

    vox_rotate_vector (camera->rotation, ray, ray);
}

static float simple_get_fov (const vox_camera *cam)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    return camera->fov;
}

static void simple_set_fov (vox_camera *cam, float fov)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    camera->fov = fov;
}

static void simple_get_angles (const vox_camera *cam, float *phi, float *psi)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    *phi = camera->phi;
    *psi = camera->psi;
}

static void simple_set_angles (vox_camera *cam, float phi, float psi)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    camera->phi = phi;
    camera->psi = psi;
}

static float* simple_get_position (const vox_camera *cam)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    return camera->position;
}

static void simple_set_position (vox_camera *cam, vox_dot pos)
{
    vox_simple_camera *camera = (vox_simple_camera*)cam;
    vox_dot_copy (camera->position, pos);
}

static const struct
{
    void   (*screen_2_world) (vox_camera*, vox_dot, int, int, int, int);
    float* (*camera_get_position) (const vox_camera*);
    void   (*camera_set_position) (vox_camera*, vox_dot);
    float  (*camera_get_fov) (const vox_camera*);
    void   (*camera_set_fov) (vox_camera*, float);
    void   (*camera_get_angles) (const vox_camera*, float*, float*);
    void   (*camera_set_angles) (vox_camera*, float, float);
} cam_methods_dispatch[] =
{{simple_screen_2_world, simple_get_position, simple_set_position,
  simple_get_fov, simple_set_fov, simple_get_angles, simple_set_angles}};


float* vox_camera_get_position (const vox_camera *cam)
{
    assert ((cam->cam_type >= 0) && (cam->cam_type < VOX_CAMERA_MAX));
    return cam_methods_dispatch[cam->cam_type].camera_get_position (cam);
}

void vox_camera_set_position (vox_camera *cam, vox_dot pos)
{
    assert ((cam->cam_type >= 0) && (cam->cam_type < VOX_CAMERA_MAX));
    cam_methods_dispatch[cam->cam_type].camera_set_position (cam, pos);
}

float vox_camera_get_fov (const vox_camera *cam)
{
    assert ((cam->cam_type >= 0) && (cam->cam_type < VOX_CAMERA_MAX));
    return cam_methods_dispatch[cam->cam_type].camera_get_fov (cam);
}

void vox_camera_set_fov (vox_camera *cam, float fov)
{
    assert ((cam->cam_type >= 0) && (cam->cam_type < VOX_CAMERA_MAX));
    cam_methods_dispatch[cam->cam_type].camera_set_fov (cam, fov);
}

void vox_camera_get_angles (const vox_camera *cam, float *phi, float *psi)
{
    assert ((cam->cam_type >= 0) && (cam->cam_type < VOX_CAMERA_MAX));
    cam_methods_dispatch[cam->cam_type].camera_get_angles (cam, phi, psi);
}

void vox_camera_set_angles (vox_camera *cam, float phi, float psi)
{
    assert ((cam->cam_type >= 0) && (cam->cam_type < VOX_CAMERA_MAX));
    cam_methods_dispatch[cam->cam_type].camera_set_angles (cam, phi, psi);
}
