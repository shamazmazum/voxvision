/**
   @file camera.h
   @brief Methods for camera class and a simple camera
**/
#ifndef CAMERA_H
#define CAMERA_H

#include "../params_var.h"

/**
   \brief Camera class
**/
typedef struct
{
    void *camera;
    void (*screen2world) (void*, vox_dot, int, int, int, int);
    float* (*get_position) (void*);
    void (*get_rot_angles) (void*, float*, float*, float*);
    void (*set_rot_angles) (void*, float, float, float);
} vox_camera_interface;

typedef struct
{
    vox_camera_interface iface;

    vox_dot position; /**< \brief Position of the camera */
    float fov;        /**< \brief Field of view */
    float rotx;       /**< \brief Rotation angle around axis Ox (left-right) */
    float roty;       /**< \brief Currently unused */
    float rotz;       /**< \brief Rotation angle around axis Oz (up-down) */

    vox_quat rotation;
} vox_simple_camera;

/**
   \brief Initialize simple camera

   \param camera a simple camera
   \param fov field of view
   \param position position of the camera
**/
void vox_make_simple_camera (vox_simple_camera*, float, vox_dot);

#endif
