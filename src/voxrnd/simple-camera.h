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
    struct vox_rnd_ctx *ctx;

    vox_dot position;
    vox_quat rotation;

    float fov;
    float body_radius;
    float xmul, ymul;
    // 64-byte border (SSE)
#endif
};

/**
   \brief Get methods of the simple camera

   Its constructor takes 2 arguments. double 'fov' (field of view)
   and vox_dot pos (camera position).
**/
struct vox_camera_interface* vox_simple_camera_iface ();

/**
   \brief Set body radius of a simple camera.

   A simple camera has a spherical body in the space.
   This body cannot intersect any other object,
   so this way collision detection is implemented.
   This function sets a radius of this sphere.

   \return A new body radius.
   If supplied radius is lesser than zero, zero is returned
**/
float vox_simple_camera_set_radius (struct vox_simple_camera *camera, float radius);

/**
   \brief Get body radius of a simple camera.

   See vox_simple_camera_set_radius() for details.
**/
float vox_simple_camera_get_radius (struct vox_simple_camera *camera);

#endif