/**
   @file camera.h
   @brief Camera interface

   See vox_camera_interface structure to get more info on camera interface.
**/
#ifndef CAMERA_H
#define CAMERA_H

#include "object.h"

struct vox_camera;

/**
   \brief A camera user interface
**/
struct vox_camera_interface
{
    VOX_OBJECT (vox_camera);
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

    void (*rotate_camera) (struct vox_camera *camera, const vox_dot delta);
    /**<
       \brief Rotate the camera.

       Deltas of rotation angles are in the camera coordinate system.
       Positive direction of axis Ox is to the camera's right, of axis
       Oy is to the front, of axis Oz is up.

       \param delta a vector with deltas of rotation angles. Must contain 3
              elements.
    */

    void (*move_camera) (struct vox_camera *camera, const vox_dot delta);
    /**<
       \brief Move the camera.

       \param delta a vector with deltas of camera position coordinates (in
              camera's coordinate system). Must contain 3 elements.
    */

    void (*look_at) (struct vox_camera *camera, const vox_dot coord);
    /**<
       \brief Look at the object.

       Turn the camera so that the object with given coordinates will be in
       center of the screen.
    */

    /*
     * ------------------------
     * Specific setters/getters
     * ------------------------
     */
    void (*get_position) (const struct vox_camera *camera, vox_dot res);
    /**<
       \brief Get camera position.

       Camera position is copied to function's argument.

       \param res where result is stored
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
     * --------------
     * 64-byte border
     * --------------
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

    void (*destroy_camera) (struct vox_camera *camera);
    /**<
       \brief Destroy camera after use.

       This method should be called when camera is no longer needed.
    */
};

/**
   \brief A generic camera class.

   User is only allowed to access its methods.
   Also user can set the following properties of a camera:

   `"fov"` which stands for "field of view" with the method
   `set_property_number`.

   `"position"` which is position of the camera with the method
   `set_property_dot`.

   `"rotation"` which is rotation angles of the camera with the method
   `set_property_dot`.
**/
struct vox_camera
{
    struct vox_camera_interface *iface; /**< \brief camera methods **/
};

/**
   \brief Use defined camera methods from another interface implementation.

   This functions overwrites camera methods of the given camera with new methods
   from interface `iface` if they are defined (that means they hold non-NULL
   value).
**/
void vox_use_camera_methods (struct vox_camera *camera, const struct vox_camera_interface *iface);

/**
   \brief Initialize camera.

   This function allocates space for and initializes camera's `iface` field with
   dummy methods, so you do not have to write an implementation for EVERY camera
   method to implement the interface. You can implement SOME methods and use
   `vox_use_camera_methods` to overwrite dummy methods later.

   Also, dummy interface implements `destroy_camera` method that calls `free` on
   the caller's `iface` field and the caller itself.
**/
void vox_init_camera (struct vox_camera *camera);

/**
   \brief Get camera methods.

   If needed, the camera is loaded as a shared object from local or global
   module directories or from directory specified in `VOXVISION_MODULES`
   environment variable. To get a pointer to the camera's methods, a function
   `module_init()` is called from that shared object.

   Now this function uses generic module mechanism provided in `modules.h`.

   \return A pointer to the camera's methods or NULL if no camera module was
           found.
**/
struct vox_camera_interface* vox_camera_methods (const char *name);
#endif
