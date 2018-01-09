#include <stdlib.h>
#include "lights.h"
#include "../voxtrees/geom.h"

struct vox_light_manager {
    vox_dot ambient_light;
    struct vox_mtree_node *bound_lights;
};

static int check_light (const vox_dot color);

struct vox_light_manager* vox_create_light_manager ()
{
    struct vox_light_manager *light_manager = malloc (sizeof (struct vox_light_manager));
    light_manager->bound_lights = NULL;
    vox_dot_set (light_manager->ambient_light, 0, 0, 0);

    return light_manager;
}

void vox_destroy_light_manager (struct vox_light_manager *light_manager)
{
    free (light_manager);
}

int vox_set_ambient_light (struct vox_light_manager *light_manager,
                            const vox_dot color)
{
    int res = 0;

    if (check_light (color)) {
        res = 1;
        vox_dot_copy (light_manager->ambient_light, color);
    }

    return res;
}

int vox_insert_shadowless_light (struct vox_light_manager *light_manager,
                                 const vox_dot center, float radius,
                                 const vox_dot color)
{
    if (!check_light (color)) return 0;

    struct vox_sphere s;
    vox_dot_copy (s.center, center);
    vox_dot_copy (s.color, color);
    s.radius = radius;

    return vox_mtree_add_sphere (&(light_manager->bound_lights), &s);
}

int vox_delete_shadowless_light (struct vox_light_manager *light_manager,
                                 const vox_dot center, float radius)
{
    struct vox_sphere s;
    vox_dot_copy (s.center, center);
    s.radius = radius;

    return vox_mtree_remove_sphere (&(light_manager->bound_lights), &s);
}

int vox_shadowless_lights_number (const struct vox_light_manager *light_manager)
{
    return vox_mtree_items (light_manager->bound_lights);
}

#ifdef SSE_INTRIN
static int check_light (const vox_dot color)
{
    __v4sf min = _mm_set_ps1 (0);
    __v4sf max = _mm_set_ps1 (1);
    __v4sf c = _mm_load_ps (color);

    __v4sf c1 = c < min;
    __v4sf c2 = c > max;
    __v4sf c3 = _mm_or_ps (c1, c2);
    int mask = _mm_movemask_ps (c3);
    return !(mask & 0x7);
}

Uint32 vox_get_color (const struct vox_light_manager *light_manager,
                      const SDL_PixelFormat *format,
                      const vox_dot intersection)
{
    __block __v4sf color = _mm_load_ps (light_manager->ambient_light);
    vox_mtree_spheres_containing (light_manager->bound_lights, intersection,
                                  ^(const struct vox_sphere *s){
                                      float dist = sqrtf (vox_sqr_metric (s->center, intersection));
                                      float add = 1 - dist/s->radius;
                                      color += _mm_set_ps1 (add) * _mm_load_ps (s->color);
                                  });
    color = _mm_min_ps (color, _mm_set_ps1 (1.0));

    return SDL_MapRGB (format, 255 * color[0], 255 * color[1], 255 * color[2]);
}

#else /* SSE_INTRIN */
static int check_light (const vox_dot color)
{
    int i;
    for (i=0; i<3; i++) {
        if (color[i] < 0 || color[i] > 1) return 0;
    }

    return 1;
}

Uint32 vox_get_color (const struct vox_light_manager *light_manager,
                      const SDL_PixelFormat *format,
                      const vox_dot intersection)
{
    __block float r, g, b;
    r = light_manager->ambient_light[0];
    g = light_manager->ambient_light[1];
    b = light_manager->ambient_light[2];

    vox_mtree_spheres_containing (light_manager->bound_lights, intersection,
                                  ^(const struct vox_sphere *s){
                                      float dist = sqrtf (vox_sqr_metric (s->center, intersection));
                                      float add = 1 - dist/s->radius;
                                      r += add * s->color[0];
                                      g += add * s->color[1];
                                      b += add * s->color[2];
                                  });
    r = fminf (r, 1);
    g = fminf (g, 1);
    b = fminf (b, 1);

    Uint32 color = SDL_MapRGB (format, 255*r, 255*g, 255*b);
    return color;
}
#endif
