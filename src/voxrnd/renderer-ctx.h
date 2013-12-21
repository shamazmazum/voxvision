#ifndef RENDERER_CTX_H
#define RENDERER_CTX_H

#include <SDL/SDL.h>
#include "methods.h"
#include "../params_var.h"

typedef struct
{
    vox_dot origin;
    vox_dot dir;
    vox_dot inter;
    void *user_data;
} vox_rnd_context;

typedef struct
{
    SDL_Surface *surface;
    class_t *camera;
//    float dx, dy;
    int p;
    int sx, sy;
} vox_rnd_aux_ctx;

void vox_init_renderer_context (vox_rnd_context*, SDL_Surface*, class_t*);
void vox_free_renderer_context (vox_rnd_context*);

#endif
