#ifndef CAMERA_H
#define CAMERA_H

#include "../params_var.h"
#include "methods.h"

#define VOX_CAMERA_SIMPLE 0
//#define VOX_CAMERA_MAX 1

typedef struct
{
    int obj_type;
    
    vox_dot position;
    float fov;
    float rotx;
    float rotz;
    
    vox_quat rotation;
} vox_simple_camera;

void vox_make_simple_camera (vox_simple_camera*, float, vox_dot);
void vox_camera_screen2world (const class_t*, vox_dot, int, int, int, int);

// Getter/Setter stuff.
float* vox_camera_position_ptr (const class_t*);

GETTER_PROTO (fov, float)
SETTER_PROTO (fov, float)

GETTER_PROTO (rotx, float)
SETTER_PROTO (rotx, float)

GETTER_PROTO (roty, float)
SETTER_PROTO (roty, float)

GETTER_PROTO (rotz, float)
SETTER_PROTO (rotz, float)

#endif
