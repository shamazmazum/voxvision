/**
   @file camera.h
   @brief Methods for camera class and a simple camera
**/
#ifndef CAMERA_H
#define CAMERA_H

#include "../voxvision.h"

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
    void (*move_camera) (void*, vox_dot, int (*) (vox_dot, void*), void*);
} vox_camera_interface;

#ifdef VOXRND_SOURCE
typedef struct
{
    vox_camera_interface iface;

    vox_dot position; /**< \brief Position of the camera */
    float fov;        /**< \brief Field of view */
    float rotx;       /**< \brief Rotation angle around axis Ox (left-right) */
    float roty;       /**< \brief Currently unused */
    float rotz;       /**< \brief Rotation angle around axis Oz (up-down) */

    vox_quat rotation;
    void *space;
} vox_simple_camera;
#else
typedef struct
{
    vox_camera_interface iface;
} vox_simple_camera;
#endif

/**
   \brief Create and initialize simple camera

   Camera must be freed with vox_destroy_simple_camera() after use.

   \param fov field of view
   \param position position of the camera
**/
vox_simple_camera* vox_make_simple_camera (float, vox_dot);

/**
   \brief Destroy a simple camera and free memory

   \param camera a camera to be destroyed
**/
void vox_destroy_simple_camera (vox_simple_camera*);

#endif
