/**
   @file camera.h
   @brief Methods for camera class and a simple camera.

   See vox_camera_interface structure to get more info on camera interface.
**/
#ifndef CAMERA_H
#define CAMERA_H

#include <voxvision.h>

/**
   \brief A camera user interface
**/
struct vox_camera_interface
{
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
       \param delta a vector with deltas of rotation angles. Must contain 3
              elements.
    */

    int (*move_camera) (void* obj, vox_dot delta);
    /**<
       \brief Move the camera.

       Camera will not move if there is a collision with any object
       on scene to which camera is attached.

       \param obj a camera object
       \param delta a vector with deltas of camera position coordinates. Must
              contain 3 elements 
    */

    void (*set_window_size) (void* obj, int w, int h);
    /**<
       \brief Set screen/window size for a camera.

       This method must be called before any screen2world() calls. Usually this
       happens automatically when vox_make_renderer_context() and/or
       vox_rc_set_camera() are called.

       \param w width of the window
       \param h height of the window
    */

    void (*destroy_camera) (void *obj);
    /**<
       \brief Destroy camera after use.

       This method should be called when camera is no longer needed.
    */

    // 64 byte border. Rarely used methods are below this line.
};

/**
   \brief A camera class.

   User is only allowed to access its interface.
**/
struct vox_camera
{
    struct vox_camera_interface *iface;
#ifdef VOXRND_SOURCE
    struct vox_rnd_ctx *ctx;
#endif
};

struct vox_simple_camera
{
    struct vox_camera_interface *iface;
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
   \brief Create and initialize a simple camera

   Camera can be freed after use with destroy_camera() method.

   \param fov field of view
   \param position position of the camera
**/
struct vox_simple_camera* vox_make_simple_camera (float fov, vox_dot position);

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
