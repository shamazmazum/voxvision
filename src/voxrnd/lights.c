#include <stdlib.h>
#include "lights.h"
#include "../voxtrees/geom.h"

struct vox_light_manager {
    struct vox_mtree_node *bound_lights;
};

struct vox_light_manager* vox_create_light_manager ()
{
    struct vox_light_manager *light_manager = malloc (sizeof (struct vox_light_manager));
    light_manager->bound_lights = NULL;

    return light_manager;
}

void vox_destroy_light_manager (struct vox_light_manager *light_manager)
{
    free (light_manager);
}

// Ignore color for now
int vox_insert_shadowless_light (struct vox_light_manager *light_manager,
                                 const struct vox_sphere *position,
                                 Uint32 color)
{
    return vox_mtree_add_sphere (&(light_manager->bound_lights), position);
}

int vox_delete_shadowless_light (struct vox_light_manager *light_manager,
                                 const struct vox_sphere *position)
{
    return vox_mtree_remove_sphere (&(light_manager->bound_lights), position);
}

int vox_shadowless_lights_number (const struct vox_light_manager *light_manager)
{
    return vox_mtree_items (light_manager->bound_lights);
}

Uint32 vox_get_color (const struct vox_light_manager *light_manager,
                      const SDL_PixelFormat *format,
                      const vox_dot intersection)
{
    __block float intensity = 0.1;
    vox_mtree_spheres_containing (light_manager->bound_lights, intersection,
                                  ^(const struct vox_sphere *s){
                                      float dist = sqrtf (vox_sqr_metric (s->center, intersection));
                                      float add = 1 - dist/s->radius;
                                      intensity += add;
                                  });
    intensity = fminf (1, intensity);
    Uint32 col = 255 * intensity;

    Uint32 color = SDL_MapRGB (format, col, col, col);
    return color;
}
