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
    Uint32 res = SDL_MapRGB (context->surface->format, 255, 0, 0);
    return res;
}

static struct vox_rnd_ctx* allocate_context () {
    struct vox_rnd_ctx *ctx = malloc (sizeof (struct vox_rnd_ctx));
    memset (ctx, 0, sizeof (*ctx));
    ctx->quality = VOX_QUALITY_ADAPTIVE;

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

int vox_context_set_quality (struct vox_rnd_ctx *ctx, unsigned int quality) {
    /* Ray merge mode check */
    if ((quality & VOX_QUALITY_RM_MASK) &&
        ((quality & VOX_QUALITY_MODE_MASK) !=
         VOX_QUALITY_ADAPTIVE)) {
        return 0;
    }

    /* Check if reserved values are not set */
    if ((quality & VOX_QUALITY_MODE_MASK) >
        VOX_QUALITY_MODE_MAX) {
        return 0;
    }

    if ((quality & VOX_QUALITY_RM_MASK) >
        VOX_QUALITY_RM_MAX) {
        return 0;
    }

    ctx->quality = quality;
    return 1;
}

#define LENGTH_THRESHOLD 1.69
#define MAX_DIST 150

void vox_render (struct vox_rnd_ctx *ctx) {
    square *output = ctx->square_output;
    struct vox_camera *camera = ctx->camera;
    int ws = ctx->ws;
    int quality = ctx->quality;
    int rnd_mode = quality & VOX_QUALITY_MODE_MASK;
    int merge_mode = quality & VOX_QUALITY_RM_MASK;

    /*
      Render the scene running multiple tasks in parallel. Each task renders a
      square of 4x4 pixels. Inside each task we try to render the next pixel
      using previous leaf node, not root scene node, if possible.
    */
    dispatch_apply (ctx->squares_num,
                    dispatch_get_global_queue (DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                    ^(size_t cs) {
                        const struct vox_node *corner1, *corner2;
                        vox_dot dir1, dir2;
                        vox_dot inter1, inter2;
                        const struct vox_node *leaf = NULL;
                        vox_dot origin;
                        int block_rnd_mode = rnd_mode;

                        int i, xstart, ystart;
                        ystart = cs / ws;
                        xstart = cs % ws;
                        ystart <<= 2; xstart <<= 2;

                        int istart = 0, iend = 16;
                        Uint32 color;

#ifdef STATISTICS
                        const struct vox_node *old_leaf;
                        int leafs_changed = 0;
#endif

                        origin = camera->iface->get_position (camera);
                        WITH_STAT (VOXRND_BLOCKS_TRACED());
                        int block_merge_mode = 0;

                        if (block_rnd_mode == VOX_QUALITY_ADAPTIVE) {
                            block_merge_mode = (merge_mode == VOX_QUALITY_RAY_MERGE);
                            /*
                             * Here we choose which mode is actually used for rendering a block. To
                             * put it simple: choose upper-left and bottom-right pixels in the
                             * block. If intersections in those points are farther than allowed,
                             * choose "best" quality, else choose "fast". Allowed distance is
                             * depending on the camera's field of view and hard-coded threshold
                             * value, LENGTH_THRESHOLD.
                             */
                            istart = 1;
                            dir1 = camera->iface->screen2world (camera, xstart, ystart);
                            corner1 = vox_ray_tree_intersection (ctx->scene, origin, dir1,
                                                                 &inter1);
                            block_rnd_mode = VOX_QUALITY_FAST;
                            leaf = corner1;
                            if (corner1 != NULL) {
                                iend = 15;
                                /* Since we are already there, draw a pixel now. */
                                color = get_color (ctx, inter1);
                                output[cs][0] = color;
                                dir2 = camera->iface->screen2world (camera, xstart + 3, ystart + 3);
                                corner2 = vox_ray_tree_intersection (ctx->scene, origin, dir2, &inter2);
                                if (corner2 != NULL) {
                                    color = get_color (ctx, inter2);
                                    output[cs][15] = color;
                                    float d1 = vox_sqr_norm (inter1 - origin);
                                    float d2 = vox_sqr_norm (dir1);
                                    float criteria = d1 / d2 * vox_sqr_norm (dir1 - dir2);
                                    float dist = vox_sqr_norm (inter1 - inter2);
                                    if (dist/criteria > LENGTH_THRESHOLD) {
                                        block_rnd_mode = VOX_QUALITY_BEST;
                                        WITH_STAT (VOXRND_CANCELED_PREDICTION());
                                    }

                                    if (d1 > MAX_DIST * MAX_DIST) {
                                        block_merge_mode = (block_rnd_mode == VOX_QUALITY_FAST &&
                                                            merge_mode == VOX_QUALITY_RAY_MERGE_ACCURATE)? 1:
                                            block_merge_mode;
                                    } else block_merge_mode = 0;
                                }
                            }
                        }
                        WITH_STAT (if (block_merge_mode) VOXRND_RAYMERGE_BLOCK ());

                        int prev_p = 0;
                        /* istart and iend have been adjusted to a not yet drawn region. */
                        for (i=istart; i<iend; i++) {
                            int p = rendering_order[i];
                            int y = p/4;
                            int x = p%4;
                            int merge = block_merge_mode && (i & 1);

                            dir1 = camera->iface->screen2world (camera, x+xstart, y+ystart);
                            if (!merge) {
                                if (block_rnd_mode == VOX_QUALITY_FAST) {
                                    WITH_STAT (old_leaf = leaf);
                                    if (leaf != NULL) {
                                        leaf = vox_ray_tree_intersection (leaf,  origin, dir1, &inter1);
                                    }
                                    if (leaf == NULL) {
                                        leaf = vox_ray_tree_intersection (ctx->scene, origin, dir1, &inter1);
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
                                    leaf = vox_ray_tree_intersection (ctx->scene, origin, dir1, &inter1);
                                }
                            }

                            if (leaf != NULL) {
                                color = (merge)? output[cs][prev_p]: get_color (ctx, inter1);
                                output[cs][p] = color;
                            }
                            prev_p = p;
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
