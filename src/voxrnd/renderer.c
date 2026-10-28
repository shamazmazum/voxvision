#ifdef USE_GCD
#include <dispatch/dispatch.h>
#else
#include "../gcd-stubs.c"
#endif
#include <stdlib.h>
#include <assert.h>
#include <vn3d/vn3d.h>

#include "renderer.h"
#include "copy-helper.h"
#include "probes.h"
#include "../voxtrees/search.h"
#include "../voxtrees/geom.h"

/*
 * Pixels on the screen are rendered by 4x4 blocks. In case you use GCD, each
 * block is scheduled to a separate CPU core. Inside each block the rendering is
 * performed in Z-order which is encoded by following numbers. Note, that each
 * block is then written to an object called 'square' which requires 64 bytes of
 * memory (i.e. size of L1 cache line).
 */
static int rendering_order[] = {
    0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
};

static Uint32 get_color (const struct vox_rnd_ctx *context, vox_dot inter) {
    Uint32 res;
    vox_dot aligned = __builtin_elementwise_round (inter/vox_voxel) * vox_voxel;
    vox_dot diff = __builtin_elementwise_abs (inter - aligned);
    float min = fminf (diff.x, fminf (diff.y, diff.z));

    if (min == diff.x) {
        res = SDL_MapRGB (context->surface->format, 185, 155, 130);
    } else if (min == diff.y) {
        res = SDL_MapRGB (context->surface->format, 170, 170, 145);
    } else {
        res = SDL_MapRGB (context->surface->format, 190, 110, 60);
    }
    return res;
}

static struct vox_rnd_ctx* allocate_context () {
    struct vox_rnd_ctx *ctx = malloc (sizeof (struct vox_rnd_ctx));
    memset (ctx, 0, sizeof (*ctx));
    ctx->quality = VOX_QUALITY_FAST;

    return ctx;
}

static void allocate_squares (struct vox_rnd_ctx *ctx) {
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
static int bad_geometry (unsigned int width, unsigned int height) {
    return (width&0xf) || (height&0x3);
}

struct vox_rnd_ctx* vox_make_context_from_surface (SDL_Surface *surface) {
    if (bad_geometry (surface->w, surface->h)) {
        return NULL;
    }

    struct vox_rnd_ctx *ctx = allocate_context ();
    ctx->surface = surface;
    allocate_squares (ctx);

    return ctx;
}

struct vox_rnd_ctx* vox_make_context_and_window (unsigned int width, unsigned int height) {
    struct vox_rnd_ctx *ctx = NULL;

    if (bad_geometry (width, height)) {
        goto failure;
    }

    if (!(SDL_WasInit (0) & SDL_INIT_VIDEO)) {
        goto failure;
    }

    ctx = allocate_context ();
    ctx->window = SDL_CreateWindow ("voxrnd window",
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    width, height, 0);
    if (ctx->window == NULL) {
        goto failure;
    }

    ctx->surface = SDL_GetWindowSurface (ctx->window);
    if (ctx->surface == NULL) {
        goto failure;
    }

    /* Sorry, only native pixel format by now */
    assert (ctx->surface->format->BitsPerPixel == 32);
    allocate_squares (ctx);

    /* This is required on Wayland to forbid the window to be resizable. */
    SDL_SetWindowMinimumSize(ctx->window, width, height);
    return ctx;

failure:
    if (ctx != NULL) {
        vox_destroy_context (ctx);
    }

    return NULL;
}

void vox_destroy_context (struct vox_rnd_ctx *ctx) {
    if (ctx->window != NULL) {
        SDL_DestroyWindow (ctx->window);
        ctx->surface = NULL; // We do not need to free it manually
    }

    if (ctx->surface != NULL) {
        SDL_FreeSurface(ctx->surface);
    }

    free (ctx->square_output);
    free (ctx);
}

void vox_context_set_camera (struct vox_rnd_ctx *ctx, struct vox_camera *camera) {
    ctx->camera = camera;
    camera->iface->set_window_size (camera, ctx->surface->w, ctx->surface->h);
}

void vox_context_set_scene (struct vox_rnd_ctx *ctx, struct vox_node *scene) {
    ctx->scene = scene;
}

bool vox_context_set_quality (struct vox_rnd_ctx *ctx, unsigned int quality) {
    bool res;

    if ((quality & VOX_QUALITY_MODE_MASK) != quality) {
        res = false;
    } else {
        ctx->quality = quality;
        res = true;
    }

    return res;
}

void vox_render (struct vox_rnd_ctx *ctx) {
    square *output = ctx->square_output;
    struct vox_camera *camera = ctx->camera;
    int ws = ctx->ws;
    int rnd_mode = ctx->quality & VOX_QUALITY_MODE_MASK;

    /*
      Render the scene running multiple tasks in parallel. Each task renders a
      square of 4x4 pixels. Inside each task we try to render the next pixel
      using previous leaf node, not root scene node, if possible.
    */
    dispatch_apply (ctx->squares_num,
                    dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                    ^(size_t cs) {
                        vox_dot dir, inter, origin;
                        const struct vox_node *leaf = NULL;
                        Uint32 color;
                        int xstart, ystart;
                        ystart = cs / ws;
                        xstart = cs % ws;
                        ystart <<= 2; xstart <<= 2;
#ifdef STATISTICS
                        const struct vox_node *old_leaf;
                        int leafs_changed = 0;
#endif
                        origin = camera->iface->get_position (camera);
                        WITH_STAT (VOXRND_BLOCKS_TRACED());

                        for (int i = 0; i < 16; i++) {
                            int p = rendering_order[i];
                            int y = p/4;
                            int x = p%4;

                            dir = camera->iface->screen2world (camera, x+xstart, y+ystart);
                            if (rnd_mode == VOX_QUALITY_FAST) {
                                WITH_STAT (old_leaf = leaf);
                                if (leaf != NULL) {
                                    leaf = vox_ray_tree_intersection (leaf, origin, dir, &inter);
                                }
                                if (leaf == NULL) {
                                    leaf = vox_ray_tree_intersection (ctx->scene, origin, dir, &inter);
#ifdef STATISTICS
                                    if (old_leaf != NULL) {
                                        if (leaf != NULL) {
                                            VOXRND_LEAF_MISPREDICTION();
                                        } else {
                                            VOXRND_IGNORED_PREDICTION();
                                        }
                                        leafs_changed++;
                                    }
#endif
                                }
                            } else {
                                leaf = vox_ray_tree_intersection (ctx->scene, origin, dir, &inter);
                            }

                            if (leaf != NULL) {
                                color = get_color (ctx, inter);
                                output[cs][p] = color;
                            }
                        }
                        WITH_STAT (VOXRND_BLOCK_LEAFS_CHANGED (leafs_changed));
                    });
}

void vox_redraw (struct vox_rnd_ctx *ctx) {
    copy_squares (ctx->square_output, ctx->surface->pixels, ctx->ws, ctx->hs);
    memset (ctx->square_output, 0, ctx->squares_num*sizeof(square));

    if (ctx->window != NULL) {
        SDL_UpdateWindowSurface (ctx->window);
    }
}
