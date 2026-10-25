#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../voxtrees/geom.h"
#include "vect-ops.h"
#include "camera.h"
#include "modules.h"

struct vox_simple_camera {
    struct vox_camera_interface *iface; /**< \brief Simple camera methods **/
    float xsub, ysub;
    vox_dot position;
    vox_quat rotation;
    float mul, fov;
};

struct vox_module* module_init();

static vox_dot simple_screen2world (const struct vox_camera *cam, int sx, int sy) {
    const struct vox_simple_camera *camera = (void*)cam;
    float mul = camera->mul;
    float xsub = camera->xsub;
    float ysub = camera->ysub;

    assert (mul != 0 && xsub != 0 && ysub != 0);
    vox_dot ray = (vox_dot) {mul*sx - xsub, 1, ysub - mul*sy};
    return vox_rotate_vector (camera->rotation, ray);
}

static vox_dot simple_get_position (const struct vox_camera *cam) {
    const struct vox_simple_camera *camera = (void*)cam;
    return camera->position;
}

static void simple_move_camera (struct vox_camera *cam, const vox_dot delta) {
    struct vox_simple_camera *camera = (void*)cam;

    vox_dot delta_rot = vox_rotate_vector (camera->rotation, delta);
    camera->position += delta_rot;
}

static void simple_rotate_camera (struct vox_camera *cam, const vox_dot delta) {
    struct vox_simple_camera *camera = (void*)cam;
    // Basis unit vector in the camera's coordinate system
    static const vox_dot orts[] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0}
    };

    /*
      From axis Z to axis X throug axis Y.
      Rotating in that order allows simple_look_at() reuse this method.
    */
    for (int i = 2; i >= 0; i--) {
        /*
          After rotation ort[i] is the same vector but in the world coordinate
          system.
        */
        vox_dot ort_rotated = vox_rotate_vector (camera->rotation, orts[i]);
        /*
         * NB: We divide angles by 2 because rotation function rotates
         * by doubled angle.
         */
        float sinang = sinf(delta[i]/2);
        float cosang = cosf(delta[i]/2);

        ort_rotated = sinang*ort_rotated;
        ort_rotated.w = cosang;
        /*
         * Now, ort_rotated is a rotation around camera's i-th axis in the world
         * coordinate system. Apply this rotation by multiplying tmp and
         * camera->rotation quaternions.
        */
        camera->rotation = vox_quat_mul (ort_rotated, camera->rotation);
    }

    /*
     * KLUDGE: Normalize resulting quaternion.
     * Ordinary, we multiply rotation quaternions hence norm of the result is
     * always 1. But there is a cummulative computational error because I use
     * previous rotation to calculate the next. In order to fix it, I must
     * normalize.
     */
    camera->rotation = vox_quat_normalize (camera->rotation);
}

static void simple_look_at (struct vox_camera *cam, const vox_dot coord) {
    struct vox_simple_camera *camera = (void*)cam;

    vox_dot sub = coord - camera->position;

    // Reset rotation
    camera->rotation = (vox_quat) {0, 0, 0, 1};
    vox_dot rot =
        (vox_dot) { atan2f (sub[2], hypotf (sub[0], sub[1])), 0, -atan2f (sub[0], sub[1]) };
    cam->iface->rotate_camera (cam, rot);
}

static void simple_set_window_size (struct vox_camera *cam, int w, int h) {
    struct vox_simple_camera *camera = (void*)cam;

    if (w > h) {
        camera->xsub = camera->fov;
        camera->ysub = camera->fov*h/w;
        camera->mul  = camera->fov*2/w;
    } else {
        camera->ysub = camera->fov;
        camera->xsub = camera->fov*w/h;
        camera->mul  = camera->fov*2/h;
    }
}

static struct vox_camera* simple_construct_camera (const struct vox_camera *cam) {
    struct vox_simple_camera *camera;
    const struct vox_simple_camera *old_camera = (void*)cam;

    camera = vox_alloc (sizeof (struct vox_simple_camera));

    if (old_camera != NULL) {
        memcpy (camera, old_camera, sizeof (struct vox_simple_camera));
    } else {
        memset (camera, 0, sizeof (struct vox_simple_camera));
        vox_init_camera ((struct vox_camera*)camera);
        camera->fov = 1.0;
        camera->rotation = (vox_quat) {0, 0, 0, 1};
    }

    vox_use_camera_methods ((struct vox_camera*)camera,
                            (struct vox_camera_interface*)module_init()->methods);
    return (struct vox_camera*)camera;
}

static void simple_set_property_number (struct vox_camera *cam, const char *name, float value) {
    struct vox_simple_camera *camera = (struct vox_simple_camera*)cam;
    if (strcmp (name, "fov") == 0) {
        camera->fov = value;
    }
}

static void simple_set_property_dot (struct vox_camera *cam, const char *name, const vox_dot value) {
    struct vox_simple_camera *camera = (struct vox_simple_camera*)cam;
    if (strcmp (name, "position") == 0) {
        camera->position = value;
    } else if (strcmp (name, "rotation") == 0) {
        vox_quat r[3];
        /*
         * NB: We divide angles by 2 because rotation function rotates
         * by doubled angle.
         */
        r[0] = (vox_quat) { sinf (value[0]/2), 0, 0, cosf (value[0]/2) };
        r[1] = (vox_quat) { 0, sinf (value[1]/2), 0, cosf (value[1]/2) };
        r[2] = (vox_quat) { 0, 0, sinf (value[2]/2), cosf (value[2]/2) };

        camera->rotation = vox_quat_mul (r[2], vox_quat_mul (r[1], r[0]));
    }
}

static struct vox_camera_interface vox_simple_camera_interface = {
    .screen2world        = simple_screen2world,
    .get_position        = simple_get_position,
    .move_camera         = simple_move_camera,
    .rotate_camera       = simple_rotate_camera,
    .look_at             = simple_look_at,
    .set_window_size     = simple_set_window_size,
    .construct_camera    = simple_construct_camera,
    .set_property_number = simple_set_property_number,
    .set_property_dot    = simple_set_property_dot,
};

static struct vox_module vox_simple_camera_module = {
    .type    = CAMERA_MODULE,
    .methods = (struct vox_module_methods*)&vox_simple_camera_interface
};

struct vox_module* module_init () {
    return &vox_simple_camera_module;
}
