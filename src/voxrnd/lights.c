#include <stdlib.h>
#include <math.h>
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

struct vox_light_manager* vox_make_light_manager ()
{
    struct vox_light_manager *light_manager = malloc (sizeof (struct vox_light_manager));
    light_manager->bound_lights = NULL;
    vox_dot_set (light_manager->ambient_light, 1, 1, 1);

    return light_manager;
}

void vox_destroy_light_manager (struct vox_light_manager *light_manager)
{
    vox_mtree_destroy (light_manager->bound_lights);
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

void vox_delete_shadowless_lights (struct vox_light_manager *light_manager)
{
    vox_mtree_destroy (light_manager->bound_lights);
    light_manager->bound_lights = NULL;
}

int vox_shadowless_lights_number (const struct vox_light_manager *light_manager)
{
    return vox_mtree_items (light_manager->bound_lights);
}

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

    float dist = fminf (sphere->radius, sqrtf (vox_sqr_metric (sphere->center, gcs->intersection)));
    float add = 1 - dist/sphere->radius;

    float r = gcs->color [0] +  add * sphere->color[0];
    float g = gcs->color [1] +  add * sphere->color[1];
    float b = gcs->color [2] +  add * sphere->color[2];
    vox_dot_set (gcs->color, r, g, b);
}

void vox_get_light (const struct vox_light_manager *light_manager,
                    const vox_dot intersection,
                    vox_dot light)
{
    struct get_color_struct gcs;
    vox_dot_copy (gcs.color, light_manager->ambient_light);
    vox_dot_copy (gcs.intersection, intersection);

    vox_mtree_spheres_containing_f (light_manager->bound_lights, intersection, get_color_callback,
                                    &gcs);

    vox_dot_copy (light, gcs.color);
}
