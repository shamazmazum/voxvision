/**
   @file camera.h
   @brief Methods for camera class and a simple camera
**/
#ifndef CAMERA_H
#define CAMERA_H

#include "../voxvision.h"

/**
   \brief A camera user interface
**/
typedef struct
{
    void *camera; /**< \brief A reference to camera. Supply it as the first argument to camera methods */
    struct vox_rnd_ctx *ctx;

    void (*screen2world) (void* obj, vox_dot ray, int sx, int sy);
/**<
   \brief Translate screen coordinated to a direction vector.

   \param obj a camera object
   \param ray the result is stored in this vector
   \param sx screen x coordinate
   \param sy screen y coordinate
 */

    float* (*get_position) (void* obj);
    /**<
       \brief Get camera position.

       \param obj a camera object
       \return a pointer to camera position vector
    */

    void (*get_rot_angles) (void* obj, float* rotx, float* roty, float* rotz);
    /**< \brief get camera rotation angles */

    void (*set_rot_angles) (void* obj, float rotx, float roty, float rotz);
    /**< \brief set camera rotation angles */

    void (*rotate_camera) (void* obj, vox_dot delta);
    /**<
       \brief Rotate the camera.

       \param obj a camera object
       \param delta a vector with deltas of rotation angles. Must contain 3 elements
    */

    void (*move_camera) (void* obj, vox_dot delta);
    /**<
       \brief Move the camera.

       Camera will not move if there is a collision with any object
       on scene to which camera is attached.

       \param obj a camera object
       \param delta a vector with deltas of camera position coordinates. Must contain 3 elements
    */
} vox_camera_interface;

/**
   \brief A camera class.

   User must not touch it. Use vox_camera_interface instead.
**/
#ifdef VOXRND_SOURCE
typedef struct
{
    vox_camera_interface iface;

    vox_dot position; /**< \brief Position of the camera */
    float fov;        /**< \brief Field of view */
    float rotx;       /**< \brief Rotation angle around axis Ox (up-down) */
    float roty;       /**< \brief Rotation angle around axis Oy (left-right) */
    float rotz;       /**< \brief Currently unused */

    vox_quat rotation;
} vox_simple_camera;
#else
typedef struct
{
    vox_camera_interface iface; /**< \brief camera interface */
} vox_simple_camera;
#endif

/**
   \brief Create and initialize a simple camera

   Camera must be free()'d after use. It also
   must become a part of one renderer context
   before any use.

   \param fov field of view
   \param position position of the camera
**/
vox_simple_camera* vox_make_simple_camera (float fov, vox_dot position);

#if 0
/**
   \brief Destroy a simple camera and free its memory

   \param camera a camera to be destroyed
**/
void vox_destroy_simple_camera (vox_simple_camera* camera);
#endif

#endif
