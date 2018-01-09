#ifndef __LIGHTS_H__
#define __LIGHTS_H__
#include <SDL2/SDL.h>
#include "mtree.h"

struct vox_light_manager;

struct vox_light_manager* vox_create_light_manager ();
void vox_destroy_light_manager (struct vox_light_manager *light_manager);

int vox_insert_shadowless_light (struct vox_light_manager *light_manager,
                                 const vox_dot center, float radius,
                                 const vox_dot color);

int vox_delete_shadowless_light (struct vox_light_manager *light_manager,
                                 const vox_dot center, float radius);

int vox_shadowless_lights_number (const struct vox_light_manager *light_manager);

int vox_set_ambient_light (struct vox_light_manager *light_manager,
                           const vox_dot color);

#ifdef VOXRND_SOURCE
Uint32 vox_get_color (const struct vox_light_manager *light_manager,
                      const SDL_PixelFormat *format,
                      const vox_dot intersection);

#endif

#endif
