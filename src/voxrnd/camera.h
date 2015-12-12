/**
   @file camera.h
   @brief Methods for camera class and a simple camera.

   See vox_camera_interface structure to get more info on camera interface.
**/
#ifndef CAMERA_H
#define CAMERA_H

#include "../voxvision.h"

/**
   \brief A camera user interface
**/
typedef struct
{
    #ifdef VOXRND_SOURCE
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
    #else
    void *padding[3];
    #endif

    float* (*get_position) (void* obj);
    /**<
       \brief Get camera position.

       \param obj a camera object
       \return a pointer to camera position vector
    */

    int (*set_position) (void *obj, vox_dot pos);
    /**<
       \brief Set camera position.

       Camera will not move if there is a collision with any object
       on scene to which camera is attached.

       \param obj a camera object
       \param pos a new position
       \return 0 on success (if no collisions is found)
    */

    void (*set_rot_angles) (void* obj, vox_dot angles);
    /**< \brief set camera rotation angles

         Rotation angles are in the world coordinate system
     */

    void (*rotate_camera) (void* obj, vox_dot delta);
    /**<
       \brief Rotate the camera.

       Deltas of rotation angles are in the camera coordinate system.
       Positive direction of axis Ox is to the camera's right, of axis
       Oy is to the front, of axis Oz is up.

       \param obj a camera object
       \param delta a vector with deltas of rotation angles. Must contain 3 elements
    */

    int (*move_camera) (void* obj, vox_dot delta);
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

    vox_dot position;
    float fov;
    float body_radius;

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

   Camera can be free()'d after use.

   \param fov field of view
   \param position position of the camera
**/
vox_simple_camera* vox_make_simple_camera (float fov, vox_dot position);

/**
   \brief Set body radius of a simple camera.

   A simple camera has a spherical body in the space.
   This body cannot intersect any other object,
   so this way collision detection is implemented.
   This function sets a radius of this sphere.
**/
void vox_simple_camera_set_radius (vox_simple_camera *camera, float radius);

#if 0
/**
   \brief Destroy a simple camera and free its memory

   \param camera a camera to be destroyed
**/
void vox_destroy_simple_camera (vox_simple_camera* camera);
#endif

#endif
