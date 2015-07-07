#include <dispatch/dispatch.h>
#include <stdlib.h>
#include "renderer.h"
#include "../voxtrees/search.h"

struct vox_rnd_ctx
{
    SDL_Surface *surface;
    struct vox_node *scene;
    vox_camera_interface *camera;

    float mul[3];
    float add[3];
};

static void color_coeff (const struct vox_node *tree, float mul[], float add[])
{
    int i;
    vox_dot min;
    vox_dot max;
    vox_bounding_box (tree, min, max);
    
    for (i=0; i<3; i++)
    {
        mul[i] = 255 / (max[i] - min[i]);
        add[i] = -255 * min[i] / (max[i] - min[i]);
    }
}

static Uint32 get_color (SDL_PixelFormat *format, vox_dot inter, float mul[], float add[])
{
    Uint8 r = mul[0]*inter[0]+add[0];
    Uint8 g = mul[1]*inter[1]+add[1];
    Uint8 b = mul[2]*inter[2]+add[2];
    Uint32 color = SDL_MapRGB (format, r, g, b);
    return color;
}

struct vox_rnd_ctx* vox_make_renderer_context (SDL_Surface *surface, struct vox_node *scene,
                                               vox_camera_interface *camera)
{
    struct vox_rnd_ctx *ctx = malloc (sizeof(struct vox_rnd_ctx));
    ctx->surface = surface;
    ctx->scene = scene;
    ctx->camera = camera;

    // FIXME: calculate colors in runtime
    // Only a temporary solution to get a colorful output
    color_coeff (scene, ctx->mul, ctx->add);
    return ctx;
}

void vox_render (struct vox_rnd_ctx *ctx)
{
    SDL_Surface *surface = ctx->surface;
    Uint32 *pixels = surface->pixels;
    int w = surface->w;
    int h = surface->h;
    vox_camera_interface *camera = ctx->camera;

    float *origin = camera->get_position (camera->camera);

    /* const struct vox_node *leaf = NULL; */
    dispatch_apply (w*h, dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_HIGH, 0),
                    ^(size_t p){
                        vox_dot dir, inter;
                        int interp;
                        int i = p/w;
                        int j = p - i*w;
                        camera->screen2world (camera->camera, dir, w, h, j, i);
#if 0
                        interp = 0;
                        if (leaf != NULL)
                            interp = vox_ray_tree_intersection (leaf,  origin, dir, inter, NULL);
                        if (interp == 0)
                            interp = vox_ray_tree_intersection (tree, origin, dir, inter,  &leaf);
#else
                        interp = vox_ray_tree_intersection (ctx->scene, origin, dir, inter, NULL);
#endif
                        if (interp)
                        {
                            Uint32 color = get_color (surface->format, inter, ctx->mul, ctx->add);
                            pixels[p] = color;
                        }
                    });
    SDL_Flip (surface);
}
