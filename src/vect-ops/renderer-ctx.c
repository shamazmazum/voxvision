
>>> renderer-ctx.c 0000 0x00000001cdbe0700 02-Dec-2013 20:32:55

#include <stdlib.h>
#include "renderer-ctx.h"

#define RAY_DIST 400.0

void init_renderer_context (vox_rnd_context *ctx, SDL_Surface *surf, vox_camera *camera)
{
    ctx->user_data = malloc (sizeof (vox_rnd_aux_ctx));
    vox_rnd_aux_ctx *aux_ctx = ctx->user_data;

    ctx->camera = camera;
    aux_ctx->surface = surf;
    update_camera_data (camera);
}

void free_renderer_context (vox_rnd_context *ctx)
{
    free (ctx->user_data);
}
