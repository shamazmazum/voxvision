#ifdef USE_GCD
#include <dispatch/dispatch.h>
#endif
#include <stdlib.h>
#include "renderer.h"
#include "../voxtrees/search.h"

static void color_coeff (const struct vox_node *tree, float mul[], float add[])
{
    int i;
    struct vox_box bb;

    if (tree != NULL)
    {
        vox_bounding_box (tree, &bb);
        for (i=0; i<3; i++)
        {
            mul[i] = 255 / (bb.max[i] - bb.min[i]);
            add[i] = -255 * bb.min[i] / (bb.max[i] - bb.min[i]);
        }
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
                                               struct vox_camera_interface *camera)
{
    struct vox_rnd_ctx *ctx = malloc (sizeof(struct vox_rnd_ctx));
    ctx->surface = surface;
    if (camera != NULL) vox_rc_set_camera (ctx, camera);
    vox_rc_set_scene (ctx, scene);

    return ctx;
}

void vox_rc_set_camera (struct vox_rnd_ctx *ctx, struct vox_camera_interface *camera)
{
    ctx->camera = camera;
    camera->ctx = ctx;
    camera->set_window_size (camera->camera, ctx->surface->w, ctx->surface->h);
}

void vox_rc_set_scene (struct vox_rnd_ctx *ctx, struct vox_node *scene)
{
    ctx->scene = scene;
    // FIXME: calculate colors in runtime
    // Only a temporary solution to get a colorful output
    color_coeff (scene, ctx->mul, ctx->add);
}

void vox_render (struct vox_rnd_ctx *ctx)
{
    SDL_Surface *surface = ctx->surface;
    Uint32 *pixels = surface->pixels;
    int w = surface->w;
    int h = surface->h;
    struct vox_camera_interface *camera = ctx->camera;
    int n = w*h;
    float *origin = camera->get_position (camera->camera);

    /*
      Render the scene running multiple tasks in parallel.
      Each task renders a stipe 1x4 pixels (when not crossing window border).
      Inside each task we try to render the next pixel using previous leaf node,
      not root scene node, if possible
    */
#ifdef USE_GCD
    dispatch_apply (n>>2, dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                    ^(size_t p1)
#else
    int p1;
    for (p1 = 0; p1 < n>>2; p1++)
#endif
                    {
                        const struct vox_node *leaf = NULL;
                        vox_dot dir, inter;
                        int interp, p2, p;
                        p = p1 << 2;
                        for (p2=0; p2<4; p2++)
                        {
                            if (p >= n) break;
                            int i = p/w;
                            int j = p - i*w;
                            camera->screen2world (camera->camera, dir, j, i);
#if 1
                            interp = 0;
                            if ((leaf != NULL) && (leaf != ctx->scene))
                                interp = vox_ray_tree_intersection (leaf,  origin, dir, inter, NULL);
                            if (interp == 0)
                                interp = vox_ray_tree_intersection (ctx->scene, origin, dir, inter, &leaf);
#else
                            interp = vox_ray_tree_intersection (ctx->scene, origin, dir, inter, NULL);
#endif
                            if (interp)
                            {
                                Uint32 color = get_color (surface->format, inter, ctx->mul, ctx->add);
                                pixels[p] = color;
                            }
                            p++;
                        }
                    }
#ifdef USE_GCD
        );
#endif
    SDL_Flip (surface);
}
