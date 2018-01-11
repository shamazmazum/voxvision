#ifdef USE_GCD
#include <dispatch/dispatch.h>
#else
#include "../gcd-stubs.c"
#endif
#ifdef SSE_INTRIN
#include <emmintrin.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include "renderer.h"
#include "copy-helper.h"
#include "probes.h"
#include "../voxtrees/search.h"

#ifdef SSE_INTRIN
static Uint32 get_color (const struct vox_rnd_ctx *context, vox_dot inter)
{
    SDL_PixelFormat *format = context->surface->format;
    Uint32 res;

    if (context->light_manager != NULL) {
        vox_dot light;
        vox_get_light (context->light_manager, format, inter, light);
        __v4sf color = _mm_load_ps (light);
        color = _mm_min_ps (color, _mm_set_ps1 (1.0));
        color *= _mm_set_ps1 (255);

        assert (format->format == SDL_PIXELFORMAT_ARGB8888);
        __m128i i = _mm_cvtps_epi32 (color);
        __m128i mask = _mm_set_epi8 (0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                     0x80, 0x80, 0x80, 0x80, 0x80,    0,    4,    8);
        i = _mm_shuffle_epi8 (i ,mask);

        res = i[0];
    } else res = SDL_MapRGB (format, 255, 255, 255);

    return res;
}
#else
static Uint32 get_color (const struct vox_rnd_ctx *context, vox_dot inter)
{
    SDL_PixelFormat *format = context->surface->format;
    Uint32 res;

    if (context->light_manager != NULL) {
        vox_dot light;
        vox_get_light (context->light_manager, format, inter, light);

        res = SDL_MapRGB (format,
                          255 * fminf (light[0], 1.0),
                          255 * fminf (light[1], 1.0),
                          255 * fminf (light[2], 1.0));
    } else res = SDL_MapRGB (format, 255, 255, 255);

    return res;
}
#endif

static void allocate_squares (struct vox_rnd_ctx *ctx)
{
    int w,h, squares_num;

    w = ctx->surface->w >> 2;
    h = ctx->surface->h >> 2;
    squares_num = w*h;
    ctx->square_output = vox_alloc (squares_num*sizeof(square));
    ctx->squares_num = squares_num;
    ctx->ws = w;
    ctx->hs = h;
}

// FIXME: This may be only temporary solution.
static int bad_geometry (int width, int height)
{
    return (width&0xf) || (height&0x3);
}

struct vox_rnd_ctx* vox_make_context_from_surface (SDL_Surface *surface)
{
    if (bad_geometry (surface->w, surface->h)) return NULL;
    struct vox_rnd_ctx *ctx = malloc (sizeof (struct vox_rnd_ctx));
    memset (ctx, 0, sizeof (*ctx));
    ctx->surface = surface;
    ctx->type = VOX_CTX_WO_WINDOW;
    allocate_squares (ctx);

    return ctx;
}

struct vox_rnd_ctx* vox_make_context_and_window (int width, int height)
{
    struct vox_rnd_ctx *ctx;
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;

    if (bad_geometry (width, height)) return NULL;
    if (!(SDL_WasInit (0) & SDL_INIT_VIDEO)) return NULL;
    /*
     * Kludge: This is a workaround for buggy graphics stack on FreeBSD. At
     * least on FreeBSD 11.1 and mesa 17.2.2 I can get SDL initialized even
     * without X session, but it fails to load GL library.
     *
     * The next call actually is just a detector if we are in X session or not.
     *
     * By the way, it is not needed if using proprietary nvidia drivers.
     */
#ifdef __FreeBSD__
    if (SDL_GL_LoadLibrary (NULL) < 0) return NULL;
#endif
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
    allocate_squares (ctx);

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
    if (ctx->square_output != NULL) free (ctx->square_output);
}

void vox_context_set_camera (struct vox_rnd_ctx *ctx, struct vox_camera *camera)
{
    ctx->camera = camera;
    camera->iface->set_window_size (camera, ctx->surface->w, ctx->surface->h);
}

void vox_context_set_light_manager (struct vox_rnd_ctx *ctx,
                                    struct vox_light_manager *light_manager)
{
    ctx->light_manager = light_manager;
}

void vox_context_set_scene (struct vox_rnd_ctx *ctx, struct vox_node *scene)
{
    ctx->scene = scene;
}

void vox_render (struct vox_rnd_ctx *ctx)
{
    square *output = ctx->square_output;
    struct vox_camera *camera = ctx->camera;
    int ws = ctx->ws;

    /*
      Render the scene running multiple tasks in parallel.
      Each task renders a square 4x4.
      Inside each task we try to render the next pixel using previous leaf node,
      not root scene node, if possible
    */
    dispatch_apply (ctx->squares_num, dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                    ^(size_t cs) {
                        const struct vox_node *leaf = NULL;
                        WITH_STAT (const struct vox_node *old_leaf);
                        vox_dot dir, inter, origin;

                        int i, xstart, ystart;
                        ystart = cs / ws;
                        xstart = cs % ws;
                        ystart <<= 2; xstart <<= 2;

                        camera->iface->get_position (camera, origin);
                        for (i=0; i<16; i++)
                        {
                            int y = i/4;
                            int x = i%4;

                            camera->iface->screen2world (camera, dir, x+xstart, y+ystart);
                            WITH_STAT (VOXRND_PIXEL_TRACED());
#if 1
                            WITH_STAT (old_leaf = leaf);
                            if (leaf != NULL)
                                leaf = vox_ray_tree_intersection (leaf,  origin, dir, inter);
                            if (leaf == NULL) {
                                leaf = vox_ray_tree_intersection (ctx->scene, origin, dir, inter);
#ifdef STATISTICS
                                if (old_leaf != NULL)
                                {
                                    if (leaf != NULL) VOXRND_LEAF_MISPREDICTION();
                                    else VOXRND_IGNORED_PREDICTION();
                                }
#endif
                            }
#else
                            leaf = vox_ray_tree_intersection (ctx->scene, origin, dir, inter);
#endif
                            if (leaf != NULL)
                            {
                                Uint32 color = get_color (ctx, inter);
                                output[cs][i] = color;
                            }
                        }
                    });
}

void vox_redraw (struct vox_rnd_ctx *ctx)
{
    copy_squares (ctx->square_output, ctx->surface->pixels, ctx->ws, ctx->hs);
    if (ctx->type == VOX_CTX_WO_WINDOW) return;

    SDL_UpdateTexture (ctx->texture, NULL, ctx->surface->pixels, ctx->surface->pitch);
    memset (ctx->square_output, 0, ctx->squares_num*sizeof(square));
    SDL_RenderCopy (ctx->renderer, ctx->texture, NULL, NULL);
    SDL_RenderPresent (ctx->renderer);
}
