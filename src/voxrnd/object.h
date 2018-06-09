/**
   @file object.h
   @brief Common object definitions and dummy methods.
**/
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "../voxvision.h"

#ifdef VOXRND_SOURCE
void vox_method_void_dummy (void *obj, ...);
float vox_method_float_dummy (void *obj, ...);
void vox_method_dot_dummy (void *obj, vox_dot dot, ...);


#define VOX_OBJECT_METHODS .set_property_number = (void*)vox_method_void_dummy, \
        .set_property_dot = (void*)vox_method_void_dummy,
#endif

/**
   \brief Generic object interface.

   All objects in voxrnd have these two methods in their interface. With
   `set_property_number` you can set an object's property of type `float` and
   with `set_property_dot` you can set an object's property of type `vox_dot`.

   See `vox_camera_interface` and `vox_camera` for examples.
**/   
#define VOX_OBJECT(type) void (*set_property_number) (struct type *obj, const char *name, float value); \
    void (*set_property_dot) (struct type *obj, const char *name, const vox_dot value); \

/**
   \brief Generic voxrnd object.

   This is the most generic voxrnd object.
**/
struct vox_object
{
    struct vox_object_interface *iface;
/**< \brief Generic interface implementation. **/
};

/**
   \brief Generic object interface structure.
**/
struct vox_object_interface {
    VOX_OBJECT(vox_object);
};

#endif
