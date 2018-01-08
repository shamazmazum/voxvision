#ifdef USE_GCD
#include <dispatch/dispatch.h>
#else
#include "../gcd-stubs.c"
#endif
#include <stdlib.h>
#include "renderer.h"
#include "copy-helper.h"
#include "probes.h"
#include "../voxtrees/search.h"
#include "../voxtrees/geom.h"

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
    ctx->quality = VOX_QUALITY_ADAPTIVE;
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
    ctx->quality = VOX_QUALITY_ADAPTIVE;
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

void vox_context_set_scene (struct vox_rnd_ctx *ctx, struct vox_node *scene)
{
    ctx->scene = scene;
    // FIXME: calculate colors in runtime
    // Only a temporary solution to get a colorful output
    color_coeff (scene, ctx->mul, ctx->add);
}

int vox_context_set_quality (struct vox_rnd_ctx *ctx, int quality)
{
    if (quality < VOX_QUALITY_MIN || quality > VOX_QUALITY_MAX) return 0;
    ctx->quality = quality;

    return 1;
}

#define LENGTH_THRESHOLD 1.69
void vox_render (struct vox_rnd_ctx *ctx)
{
    SDL_Surface *surface = ctx->surface;
    square *output = ctx->square_output;
    struct vox_camera *camera = ctx->camera;
    int ws = ctx->ws;
    int quality = ctx->quality;

    /*
      Render the scene running multiple tasks in parallel.
      Each task renders a square 4x4.
      Inside each task we try to render the next pixel using previous leaf node,
      not root scene node, if possible
    */
    dispatch_apply (ctx->squares_num, dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                    ^(size_t cs) {
                        const struct vox_node *corner1, *corner2;
                        vox_dot dir1, dir2;
                        vox_dot inter1, inter2;
                        const struct vox_node *leaf = NULL;
                        WITH_STAT (const struct vox_node *old_leaf);
                        vox_dot origin;
                        int mode = quality;

                        int i, xstart, ystart;
                        ystart = cs / ws;
                        xstart = cs % ws;
                        ystart <<= 2; xstart <<= 2;

                        camera->iface->get_position (camera, origin);

                        if (mode == VOX_QUALITY_ADAPTIVE) {
                            /*
                             * Here we choose which mode is actually used for rendering a block. To
                             * put it simple: choose upper-left and bottom-right pixels in the
                             * block. If intersections in those points are farther than allowed,
                             * choose "best" quality, else choose "fast". Allowed distance is
                             * depending on the camera's field of view and hard-coded threshold
                             * value, LENGTH_THRESHOLD.
                             */
                            camera->iface->screen2world (camera, dir1, xstart, ystart);
                            corner1 = vox_ray_tree_intersection (ctx->scene, origin, dir1, inter1);
                            mode = VOX_QUALITY_FAST;
                            leaf = corner1;
                            if (corner1 != NULL) {
                                camera->iface->screen2world (camera, dir2, xstart + 3, ystart + 3);
                                corner2 = vox_ray_tree_intersection (ctx->scene, origin, dir2, inter2);
                                if (corner2 != NULL) {
                                    float d1 = vox_sqr_metric (inter1, origin);
                                    float d2 = vox_sqr_norm (dir1);
                                    float criteria = d1 / d2 * vox_sqr_metric (dir1, dir2);
                                    float dist = vox_sqr_metric (inter1, inter2);
                                    if (dist/criteria > LENGTH_THRESHOLD) {
                                        mode = VOX_QUALITY_BEST;
                                        WITH_STAT (VOXRND_CANCELED_PREDICTION());
                                    }
                                }
                            }
                        }

                        /*
                         * In adaptive mode, the search will be performed twice  for left-upper and
                         * right-bottom corners. Just accept this and keep the code as simple as
                         * possible.
                         */
                        for (i=0; i<16; i++) {
                            int y = i/4;
                            int x = i%4;

                            camera->iface->screen2world (camera, dir1, x+xstart, y+ystart);
                            WITH_STAT (VOXRND_PIXEL_TRACED());
                            if (mode == VOX_QUALITY_FAST) {
                                WITH_STAT (old_leaf = leaf);
                                if (leaf != NULL)
                                    leaf = vox_ray_tree_intersection (leaf,  origin, dir1, inter1);
                                if (leaf == NULL) {
                                    leaf = vox_ray_tree_intersection (ctx->scene, origin, dir1, inter1);
#ifdef STATISTICS
                                    if (old_leaf != NULL) {
                                        if (leaf != NULL) VOXRND_LEAF_MISPREDICTION();
                                        else VOXRND_IGNORED_PREDICTION();
                                    }
#endif
                                }
                            } else leaf = vox_ray_tree_intersection (ctx->scene, origin, dir1, inter1);

                            if (leaf != NULL) {
                                Uint32 color = get_color (surface->format, inter1, ctx->mul, ctx->add);
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
