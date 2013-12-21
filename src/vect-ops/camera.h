#ifndef CAMERA_H
#define CAMERA_H

#include "../params_var.h"

#define VOX_CAMERA_SIMPLE 0
#define VOX_CAMERA_MAX 1

struct vox_camera_
{
    int cam_type;
};
typedef struct vox_camera_ vox_camera;

struct vox_simple_camera_
{
    int cam_type;
    
    vox_dot position;
    float fov;
    float phi;
    float psi;
    
    vox_quat rotation;
};
typedef struct vox_simple_camera_ vox_simple_camera;

void vox_make_simple_camera (vox_simple_camera*, float, vox_dot);

// Getter/Setter stuff. Handwrite job, but I want it to be generated in future
float* vox_camera_position_ptr (const vox_camera*);

float vox_camera_get_fov (const vox_camera*);
void vox_camera_set_fov (vox_camera*, float);

void vox_camera_get_angles (const vox_camera*, float*, float*);
void vox_camera_set_angles (vox_camera*, float, float);

#endif
