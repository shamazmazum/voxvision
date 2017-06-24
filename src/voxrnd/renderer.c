#ifdef USE_GCD
#include <dispatch/dispatch.h>
#else
#include "../gcd-stubs.c"
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

struct vox_rnd_ctx* vox_make_context_from_surface (SDL_Surface *surface)
{
    struct vox_rnd_ctx *ctx = malloc (sizeof (struct vox_rnd_ctx));
    memset (ctx, 0, sizeof (*ctx));
    ctx->surface = surface;
    ctx->type = VOX_CTX_WO_WINDOW;

    return ctx;
}

struct vox_rnd_ctx* vox_make_context_and_window (int width, int height)
{
    struct vox_rnd_ctx *ctx;
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;

    if (!(SDL_WasInit (0) & SDL_INIT_VIDEO)) return NULL;
    ctx = malloc (sizeof (struct vox_rnd_ctx));
    memset (ctx, 0, sizeof (*ctx));
    if (SDL_CreateWindowAndRenderer (width, height, 0, &ctx->window, &ctx->renderer)) goto failure;
    ctx->texture = SDL_CreateTexture (ctx->renderer, SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING, width, height);
    if (ctx->texture == NULL) goto failure;
    SDL_PixelFormatEnumToMasks (SDL_PIXELFORMAT_ARGB8888, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
    ctx->surface = SDL_CreateRGBSurface (0, width, height,
                                    bpp, Rmask, Gmask, Bmask, Amask);
    if (ctx->surface == NULL) goto failure;

    ctx->type = VOX_CTX_W_WINDOW;
    return ctx;

failure:
    vox_destroy_context (ctx);
    return NULL;
}

void vox_destroy_context (struct vox_rnd_ctx *ctx)
{
    if ((ctx->surface != NULL) & (ctx->type == VOX_CTX_W_WINDOW)) SDL_FreeSurface(ctx->surface);
    if (ctx->texture != NULL) SDL_DestroyTexture (ctx->texture);
    if (ctx->renderer != NULL) SDL_DestroyRenderer (ctx->renderer);
    if (ctx->window != NULL) SDL_DestroyWindow (ctx->window);
}

void vox_context_set_camera (struct vox_rnd_ctx *ctx, struct vox_camera *camera)
{
    ctx->camera = camera;
    camera->iface->set_window_size (camera, ctx->surface->w, ctx->surface->h);
}

void vox_context_set_scene (struct vox_rnd_ctx *ctx, struct vox_node *scene)
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
    struct vox_camera *camera = ctx->camera;
    int n = w*h;

    /*
      Render the scene running multiple tasks in parallel.
      Each task renders a stipe 1x4 pixels (when not crossing window border).
      Inside each task we try to render the next pixel using previous leaf node,
      not root scene node, if possible
    */
    dispatch_apply (n>>4, dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                    ^(size_t p1) {
                        const struct vox_node *leaf;
                        vox_dot dir, inter, origin;
                        int p2, p;
                        p = p1 << 4;
                        camera->iface->get_position (camera, origin);
                        for (p2=0; p2<16; p2++)
                        {
                            if (p >= n) break;
                            int i = p/w;
                            int j = p%w;

                            camera->iface->screen2world (camera, dir, j, i);
#if 1
                            if ((p2&3) == 0) leaf = NULL;
                            else if ((leaf != NULL) && (leaf != ctx->scene))
                                leaf = vox_ray_tree_intersection (leaf,  origin, dir, inter);
                            if (leaf == NULL)
                                leaf = vox_ray_tree_intersection (ctx->scene, origin, dir, inter);
#else
                            leaf = vox_ray_tree_intersection (ctx->scene, origin, dir, inter);
#endif
                            if (leaf != NULL)
                            {
                                Uint32 color = get_color (surface->format, inter, ctx->mul, ctx->add);
                                pixels[p] = color;
                            }
                            p++;
                        }
                    });
}

int vox_redraw (struct vox_rnd_ctx *ctx)
{
    if (ctx->type == VOX_CTX_WO_WINDOW) return 0;

    SDL_UpdateTexture (ctx->texture, NULL, ctx->surface->pixels, ctx->surface->pitch);
    SDL_FillRect (ctx->surface, NULL, 0);
    SDL_RenderCopy (ctx->renderer, ctx->texture, NULL, NULL);
    SDL_RenderPresent (ctx->renderer);
    return 1;
}
