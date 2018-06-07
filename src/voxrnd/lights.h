/**
   @file lights.h
   @brief Light manager

   The light manager which supports a few types of light. This manager operates
   with color as with floating point triplet (of type `vox_dot`). The first
   number is amount of red, the second is amount of green and the third is
   amount of blue. Accepted range is [0, 1], zero meaning "null intensity" and
   one meaning "full intensity".
**/

#ifndef __LIGHTS_H__
#define __LIGHTS_H__
#include "../voxtrees/mtree.h"

/**
   \brief Light manager opaque structure.
**/
struct vox_light_manager;

/**
   \brief Insert a shadowless light.

   This function inserts a soft, shadowless light which is bound by a
   sphere. Its intensity is at its maximum in the center of that sphere and is
   lineary faded to zero on its surface.

   \param light_manager A light manager
   \param center Center of the bounding sphere
   \param radius Radius of the bounding sphere
   \param color Color of the light.
   \return 1 on success (there was no such light in this place before), 0
   otherwise.
**/
int vox_insert_shadowless_light (struct vox_light_manager *light_manager,
                                 const vox_dot center, float radius,
                                 const vox_dot color);

/**
   \brief Delete a shadowless light with given specifications.

   The specifications must match exactly to the specifications of the light you
   wish to be removed (with exception of color, which may be any).
**/
int vox_delete_shadowless_light (struct vox_light_manager *light_manager,
                                 const vox_dot center, float radius);

/**
   \brief Delete all shadowless lights.
**/
void vox_delete_shadowless_lights (struct vox_light_manager *light_manager);

/**
   \brief Return a number of shadowless lights.
**/
int vox_shadowless_lights_number (const struct vox_light_manager *light_manager);

/**
   \brief Set an ambient light.

   The ambient light is added to all visible voxels on the screen.
   \return 1 on success, 0 otherwise.
**/
int vox_set_ambient_light (struct vox_light_manager *light_manager,
                           const vox_dot color);

#ifdef VOXRND_SOURCE
struct vox_light_manager* vox_create_light_manager ();
void vox_destroy_light_manager (struct vox_light_manager *light_manager);
void vox_get_light (const struct vox_light_manager *light_manager,
                    const vox_dot intersection,
                    vox_dot light);
#endif

#endif
