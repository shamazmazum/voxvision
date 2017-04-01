/**
   @file camera.h
   @brief Camera interface

   See vox_camera_interface structure to get more info on camera interface.
**/
#ifndef CAMERA_H
#define CAMERA_H

#include <stdarg.h>
#include "../voxvision.h"

struct vox_camera;

/**
   \brief A camera user interface
**/
struct vox_camera_interface
{
    /*
     * -------
     * Methods
     * -------
    */
    void (*screen2world) (const struct vox_camera *camera, vox_dot ray, int sx, int sy);
    /**<
       \brief Translate screen coordinated to a direction vector.

       \param ray the result is stored in this vector
       \param sx screen x coordinate
       \param sy screen y coordinate
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
              contain 3 elements.
    */

    /*
     * --------------
     * Setters/getters
     * ---------------
     */
    void (*set_rot_angles) (struct vox_camera *camera, vox_dot angles);
    /**< \brief set camera rotation angles

         Rotation angles are in the world coordinate system
     */

    void (*get_position) (const struct vox_camera *camera, vox_dot res);
    /**<
       \brief Get camera position.

       Camera position is copied to function's argument.

       \param res where result is stored
    */

    int (*set_position) (struct vox_camera *camera, vox_dot pos);
    /**<
       \brief Set camera position.

       Depending on implementation, camera may not move if there is a collision
       with any object on scene to which camera is attached.

       \param pos a new position
       \return 0 on success (if no collisions is found)
    */

    float (*get_fov) (const struct vox_camera *camera);
    /**<
       \brief Get field of view.
    **/

    void (*set_fov) (struct vox_camera *camera, float fov);
    /**<
       \brief Set field of view.
    **/

    /*
     * 64 byte border. Rarely used methods are below this line.
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

    /*
     * ----------------------
     * Constructor/destructor
     * ----------------------
     */
    struct vox_camera* (*construct_camera) (const struct vox_camera *camera);
    /**<
       \brief Create a new camera object.

       If camera is supplied, its internal fields will be copied to a newly
       created camera. If data layout of the supplied camera is different from
       one being created (i.e. their classes are not related to each other),
       expect undefined behaviour. This is a class method.

       \param camera can be NULL or another camera instance.
    **/
    void (*coerce_class) (struct vox_camera *camera);
    /**<
       \brief Change class of an existing camera instance.

       If the camera's class and a new class are not related, expect undefined
       behaviour. This is a class method.
    **/

    void (*destroy_camera) (struct vox_camera *camera);
    /**<
       \brief Destroy camera after use.

       This method should be called when camera is no longer needed.
    */
};

/**
   \brief A generic camera class.

   User is only allowed to access its methods.
**/
struct vox_camera
{
    struct vox_camera_interface *iface; /**< \brief camera methods **/
#ifdef VOXRND_SOURCE
    struct vox_rnd_ctx *ctx;
#endif
};

#endif
