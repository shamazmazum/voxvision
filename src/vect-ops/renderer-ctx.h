
>>> renderer-ctx.h 0000 0x00000001cdf4b5c0 03-Dec-2013 17:42:24

#ifndef RENDERER_CTX_H
#define RENDERER_CTX_H

#include <SDL/SDL.h>
#include "camera.h"

typedef struct
{
    vox_camera *camera;
    vox_dot dir;
    vox_dot inter;
    void *user_data;
} vox_rnd_context;

typedef struct
{
    SDL_Surface *surface;
//    float dx, dy;
    int p;
    int sx, sy;
} vox_rnd_aux_ctx;

void init_renderer_context (vox_rnd_context*, SDL_Surface*, vox_camera*);
void free_renderer_context (vox_rnd_context*);

#endif
