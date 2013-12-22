#include <SDL/SDL.h>
#include "renderer.h"
#include "camera.h"
#include "local-loop.h"

// Use vox_local_loop here. Render 1x4 line by iteration
// Increments sx and p, changes dir on each step
static void line_inc (vox_rnd_context *ctx)
{
    vox_rnd_aux_ctx *aux_ctx = ctx->user_data;
    aux_ctx->p++;
    aux_ctx->sx++;

    vox_camera_screen2world (aux_ctx->camera, ctx->dir, aux_ctx->surface->w,
                             aux_ctx->surface->h, aux_ctx->sx, aux_ctx->sy); // Slow part
}

static Uint32 get_color (vox_dot inter, float mul[], float add[])
{
    return 100; // Why not?
}

static void line_action (vox_rnd_context *ctx)
{
    vox_rnd_aux_ctx *aux_ctx = ctx->user_data;
    Uint32 color = get_color (ctx->inter, NULL, NULL);
    *((Uint32*)aux_ctx->surface->pixels + aux_ctx->p) = color;
    
}

void vox_render (struct vox_node *tree, vox_rnd_context *ctx)
{
    // Init context before we start
    vox_rnd_aux_ctx *aux_ctx = ctx->user_data;
    int w = aux_ctx->surface->w;
    int h = aux_ctx->surface->h;

    aux_ctx->sx = 0; aux_ctx->sy = 0; aux_ctx->p = 0;
    vox_dot_copy (ctx->origin, vox_camera_position_ptr (aux_ctx->camera));
    vox_camera_screen2world (aux_ctx->camera, ctx->dir, w, h, 0, 0);
    
    int i,j;
    for (i=0; i<h; i++)
    {
        for (j=0; j<w; j+=4) vox_local_loop (tree, 4, line_action, line_inc, ctx);
        aux_ctx->sx = 0;
        aux_ctx->sy++;
    }
    SDL_Flip (aux_ctx->surface);
}
