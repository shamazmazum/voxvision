#ifndef CAMERA_H
#define CAMERA_H

#include "../params_var.h"

typedef struct
{
    vox_dot origin;
    float fov;
    float phi;
    float psi;

    vox_quat rotation;
} vox_camera;

void update_camera_data (vox_camera*);
void screen_2_world (vox_camera*, vox_dot, int, int, int, int);

#endif
