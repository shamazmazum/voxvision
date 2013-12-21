#include <stdlib.h>
#include "renderer-ctx.h"

void vox_init_renderer_context (vox_rnd_context *ctx, SDL_Surface *surf, class_t *camera)
{
    ctx->user_data = malloc (sizeof (vox_rnd_aux_ctx));
    vox_rnd_aux_ctx *aux_ctx = ctx->user_data;

    aux_ctx->camera = camera;
    aux_ctx->surface = surf;
}

void vox_free_renderer_context (vox_rnd_context *ctx)
{
    free (ctx->user_data);
}
