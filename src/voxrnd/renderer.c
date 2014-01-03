#include <SDL/SDL.h>
#include "renderer.h"
#include "camera.h"
#include "local-loop.h"

static void color_coeff (struct vox_node *tree, float mul[], float add[])
{
    int i;
    for (i=0; i<3; i++)
    {
        mul[i] = 255 / (tree->bb_max[i] - tree->bb_min[i]);
        add[i] = -255 * tree->bb_min[i] / (tree->bb_max[i] - tree->bb_min[i]);
    }
}

// Use vox_local_loop here. Render 1x4 line by iteration
// Increments sx and p, changes dir on each step
static int line_inc (vox_rnd_context *ctx, int i)
{
    vox_rnd_aux_ctx *aux_ctx = ctx->user_data;
    aux_ctx->p++;
    aux_ctx->sx++;

    vox_camera_screen2world (aux_ctx->camera, ctx->dir, aux_ctx->surface->w,
                             aux_ctx->surface->h, aux_ctx->sx, aux_ctx->sy); // Slow part

    if (i==3) return 0;
    else return 1;
}

static Uint32 get_color (SDL_PixelFormat *format, vox_dot inter, float mul[], float add[])
{
    Uint8 r = mul[0]*inter[0]+add[0];
    Uint8 g = mul[1]*inter[1]+add[1];
    Uint8 b = mul[2]*inter[2]+add[2];
    Uint32 color = SDL_MapRGB (format, r, g, b);
    return color;
}

static void line_action (vox_rnd_context *ctx)
{
    vox_rnd_aux_ctx *aux_ctx = ctx->user_data;
    Uint32 color = get_color (aux_ctx->surface->format, ctx->inter, aux_ctx->col_mul, aux_ctx->col_add);
    *((Uint32*)aux_ctx->surface->pixels + aux_ctx->p) = color;
    
}

void vox_render (struct vox_node *tree, vox_rnd_context *ctx)
{
    // Init context before we start
    vox_rnd_aux_ctx *aux_ctx = ctx->user_data;
    int w = aux_ctx->surface->w;
    int h = aux_ctx->surface->h;

    // FIXME: calculate colors in runtime
    // Only a temporary solution to get a colorful output
    color_coeff (tree, aux_ctx->col_mul, aux_ctx->col_add);

    aux_ctx->sx = 0; aux_ctx->sy = 0; aux_ctx->p = 0;
    vox_dot_copy (ctx->origin, vox_camera_position_ptr (aux_ctx->camera));
    vox_camera_screen2world (aux_ctx->camera, ctx->dir, w, h, 0, 0);
    
    int i,j;
    for (i=0; i<h; i++)
    {
        for (j=0; j<w; j+=4) vox_local_loop (tree, line_action, line_inc, ctx);
        aux_ctx->sx = 0;
        aux_ctx->sy++;
    }
    SDL_Flip (aux_ctx->surface);
}
