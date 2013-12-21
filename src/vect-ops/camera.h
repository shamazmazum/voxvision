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
    float phi;
    float psi;
    
    vox_quat rotation;
} vox_simple_camera;

void vox_make_simple_camera (vox_simple_camera*, float, vox_dot);

// Getter/Setter stuff.
float* vox_camera_position_ptr (const class_t*);

GETTER_PROTO (fov, float)
SETTER_PROTO (fov, float)

GETTER_PROTO (phi, float)
SETTER_PROTO (phi, float)

GETTER_PROTO (psi, float)
SETTER_PROTO (psi, float)

#endif
