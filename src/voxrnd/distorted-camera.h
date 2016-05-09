/**
   @file distorted-camera.h
   @brief Distorted camera structure and functions.

   Interface getter for distorted camera.
**/
#ifndef DISTORTED_CAMERA_H
#define DISTORTED_CAMERA_H

/**
   \brief The distorted camera class.
**/
struct vox_distorted_camera
{
    struct vox_camera_interface *iface; /**< \brief Distorted camera methods **/
};

/**
   \brief Get methods of the distorted camera.

   Its constructor is like the simple camera's constructor,
   see vox_simple_camera_iface()
**/
struct vox_camera_interface* vox_distorted_camera_iface ();

#endif
