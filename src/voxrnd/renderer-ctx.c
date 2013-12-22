#include <stdlib.h>
#include "renderer-ctx.h"

vox_rnd_context* vox_init_renderer_context (SDL_Surface *surf, class_t *camera)
{
    vox_rnd_context *ctx = malloc (sizeof (vox_rnd_context));
    vox_rnd_aux_ctx *aux_ctx = malloc (sizeof (vox_rnd_aux_ctx));
    
    ctx->user_data = aux_ctx;
    aux_ctx->camera = camera;
    aux_ctx->surface = surf;

    return ctx;
}

void vox_free_renderer_context (vox_rnd_context *ctx)
{
    free (ctx->user_data);
    free (ctx);
}
