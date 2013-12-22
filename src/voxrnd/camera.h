/**
   @file camera.h
   @brief Methods for camera class and a simple camera
**/
#ifndef CAMERA_H
#define CAMERA_H

#include "../params_var.h"
#include "methods.h"

/**
   \brief OBJ_ID for a simple camera
**/
#define VOX_CAMERA_SIMPLE 0
//#define VOX_CAMERA_MAX 1

/**
   \brief Simple camera class
**/
typedef struct
{
    int obj_type;
    
    vox_dot position; /**< \brief Position of the camera */
    float fov;        /**< \brief Field of view */
    float rotx;       /**< \brief Rotation angle around axis Ox (left-right) */
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

/**
   \brief Translate screen coordinates to world coordinates
   
   \param cam a camera object
   \param ray a vector to store the result
   \param w screen width
   \param h screen height
   \param sx screen "x"
   \param sy screen "y"
**/
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
