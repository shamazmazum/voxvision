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
    vox_dot position; /**< \brief Position of the camera */
    float fov;        /**< \brief Field of view */
    float rotx;       /**< \brief Rotation angle around axis Ox (left-right) */
    float roty;       /**< \brief Currently unused */
    float rotz;       /**< \brief Rotation angle around axis Oz (up-down) */

    // Methods (so there is a limited possibility to redefine the camera with
    //          compatible ABI, OMG):
    void (*update_rotation) (void*); /**< \brief Call this after camera rotation */
    void (*screen2world) (void*, vox_dot, int, int, int, int);

    vox_quat rotation;
} vox_camera;

/**
   \brief Initialize simple camera

   \param camera a simple camera
   \param fov field of view
   \param position position of the camera
**/
void vox_make_simple_camera (vox_camera*, float, vox_dot);

#endif
