/**
   @file camera.h
   @brief Camera interface

   See vox_camera_interface structure to get more info on camera interface.
**/
#ifndef CAMERA_H
#define CAMERA_H

#include <voxvision.h>
#include <stdarg.h>

struct vox_camera;

/**
   \brief A camera user interface
**/
struct vox_camera_interface
{
    void (*screen2world) (struct vox_camera *camera, vox_dot ray, int sx, int sy);
    /**<
       \brief Translate screen coordinated to a direction vector.

       \param ray the result is stored in this vector
       \param sx screen x coordinate
       \param sy screen y coordinate
     */

    float* (*get_position) (struct vox_camera *camera);
    /**<
       \brief Get camera position.

       \return a pointer to camera position vector
    */

    int (*set_position) (struct vox_camera *camera, vox_dot pos);
    /**<
       \brief Set camera position.

       Depending on implementation, camera may not move if there is a collision
       with any object on scene to which camera is attached.

       \param pos a new position
       \return 0 on success (if no collisions is found)
    */

    void (*set_rot_angles) (struct vox_camera *camera, vox_dot angles);
    /**< \brief set camera rotation angles

         Rotation angles are in the world coordinate system
     */

    void (*rotate_camera) (struct vox_camera *camera, vox_dot delta);
    /**<
       \brief Rotate the camera.

       Deltas of rotation angles are in the camera coordinate system.
       Positive direction of axis Ox is to the camera's right, of axis
       Oy is to the front, of axis Oz is up.

       \param delta a vector with deltas of rotation angles. Must contain 3
              elements.
    */

    int (*move_camera) (struct vox_camera *camera, vox_dot delta);
    /**<
       \brief Move the camera.

       Depending on implementation, camera may not move if there is a collision
       with any object on scene to which camera is attached.

       \param delta a vector with deltas of camera position coordinates. Must
              contain 3 elements 
    */

    void (*set_window_size) (struct vox_camera *camera, int w, int h);
    /**<
       \brief Set screen/window size for a camera.

       This method must be called before any screen2world() calls. Usually this
       happens automatically when vox_make_renderer_context() and/or
       vox_rc_set_camera() are called.

       \param w width of the window
       \param h height of the window
    */

    struct vox_camera* (*construct_camera) (struct vox_camera *camera, ...);
    /**<
       \brief Create a new camera object.

       \param camera is currently ignored and may be NULL.
       \param ... parameters passed to constructor
    **/

    // 64 byte border. Rarely used methods are below this line.

    struct vox_camera* (*vconstruct_camera) (struct vox_camera *camera, va_list args);
    /**<
       \brief Create a new camera object (va_args flavor).

       \param camera is currently ignored and may be NULL.
       \param args parameters passed to constructor
    **/

    void (*destroy_camera) (struct vox_camera *camera);
    /**<
       \brief Destroy camera after use.

       This method should be called when camera is no longer needed.
    */
};

/**
   \brief A generic camera class.

   User is only allowed to access its interface.
**/
struct vox_camera
{
    struct vox_camera_interface *iface;
#ifdef VOXRND_SOURCE
    struct vox_rnd_ctx *ctx;
#endif
};

#endif
