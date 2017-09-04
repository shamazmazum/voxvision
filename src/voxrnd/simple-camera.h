/**
   @file simple-camera.h
   @brief Simple camera structure and functions.

   Interface getter for simple camera and simple-camera-specific functions.
**/
#ifndef SIMPLE_CAMERA_H
#define SIMPLE_CAMERA_H

#include "camera.h"

/**
   \brief The simple camera class.
**/
struct vox_simple_camera
{
    struct vox_camera_interface *iface; /**< \brief Simple camera methods **/
#ifdef VOXRND_SOURCE
    float xsub, ysub;
    vox_dot position;
    vox_quat rotation;
    float mul, fov;
#endif
};

/**
   \brief Get methods of the simple camera
**/
struct vox_camera_interface* vox_simple_camera_iface ();

#endif
