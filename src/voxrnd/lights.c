#ifdef SSE_INTRIN
#include <emmintrin.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include "lights.h"
#include "../voxtrees/geom.h"

struct get_color_struct {
    vox_dot intersection;
    vox_dot color;
};

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

static void get_color_callback (const struct vox_sphere *sphere, void *arg)
{
    struct get_color_struct *gcs = arg;

    float dist = sqrtf (vox_sqr_metric (sphere->center, gcs->intersection));
    float add = 1 - dist/sphere->radius;

    __v4sf color = _mm_load_ps (gcs->color);
    color += _mm_set_ps1 (add) * _mm_load_ps (sphere->color);

    _mm_store_ps (gcs->color, color);
}

Uint32 vox_get_color (const struct vox_light_manager *light_manager,
                      const SDL_PixelFormat *format,
                      const vox_dot intersection)
{
    struct get_color_struct gcs;
    vox_dot_copy (gcs.color, light_manager->ambient_light);
    vox_dot_copy (gcs.intersection, intersection);

    vox_mtree_spheres_containing_f (light_manager->bound_lights, intersection, get_color_callback,
                                    &gcs);

    assert (format->format == SDL_PIXELFORMAT_ARGB8888);
    __v4sf color = _mm_load_ps (gcs.color);
    color = _mm_min_ps (color, _mm_set1_ps (1));
    color *= _mm_set_ps1 (255);

    __m128i i = _mm_cvtps_epi32 (color);
    __m128i mask = _mm_set_epi8 (0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                 0x80, 0x80, 0x80, 0x80, 0x80,    0,    4,    8);
    i = _mm_shuffle_epi8 (i ,mask);

    return i[0];
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

static void get_color_callback (const struct vox_sphere *sphere, void *arg)
{
    struct get_color_struct *gcs = arg;

    float dist = sqrtf (vox_sqr_metric (sphere->center, gcs->intersection));
    float add = 1 - dist/sphere->radius;

    gcs->color[0] += add * sphere->color[0];
    gcs->color[1] += add * sphere->color[1];
    gcs->color[2] += add * sphere->color[2];
}

Uint32 vox_get_color (const struct vox_light_manager *light_manager,
                      const SDL_PixelFormat *format,
                      const vox_dot intersection)
{
    struct get_color_struct gcs;
    vox_dot_copy (gcs.color, light_manager->ambient_light);
    vox_dot_copy (gcs.intersection, intersection);

    vox_mtree_spheres_containing_f (light_manager->bound_lights, intersection, get_color_callback,
                                    &gcs);

    Uint32 color = SDL_MapRGB (format,
                               255 * fminf (gcs.color[0], 1.0),
                               255 * fminf (gcs.color[1], 1.0),
                               255 * fminf (gcs.color[2], 1.0));
    return color;
}
#endif
