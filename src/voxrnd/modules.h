/**
   @file modules.h
   @brief Generic module loading facility

   This system is for internal use now, but can be used manually with
   `vox_load_module()` function.
**/
#ifndef _MODULES_H_
#define _MODULES_H_
#include "../voxvision.h"

/**
   \brief Camera module class.
**/
#define CAMERA_MODULE 0

/**
   \brief Opaque module methods structure.

   This is for type checking only. Every module class has its own data type for
   this purpose.
**/
struct vox_module_methods;

/**
   \brief Module info structure.

   Each module must have a function called `module_init()` which returns
   pointer to a value of this type. It contains the module's class and its
   methods.
**/
struct vox_module {
    int type; /**< \brief A module class. **/
    struct vox_module_methods *methods;
/**< \brief Methods of the module (actually, this field may be anything you like). **/
};

/**
   \brief Load a module.

   This function searches for a module in local (~/.voxvision) or global modules
   directory or in a directory specified in `VOXVISION_MODULES` environment
   variable. If the module is found it is loaded as a shared library and the
   function `module_init()` is called in that library. This function returns a
   pointer to an object of type `struct vox_module` which contains the module's
   class and its methods. If the class matches value of `type` variable, the
   module's methods are returned.

   \param name Name of the module.
   \param type Class of the module (e.g. `CAMERA_MODULE`).

   \return The module's methods on success or `NULL` otherwise.
**/
VOX_EXPORT struct vox_module_methods* vox_load_module (const char *name, int type);

#endif
