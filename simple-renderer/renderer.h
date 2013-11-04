#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <SDL/SDL.h>
#include "../voxtrees/tree.h"

struct renderer
{
    SDL_Surface *surf;
    vox_dot origin, dir;
    float fov;

    float cmul[3];
    float cadd[3];
    float dx, dy;
};

void init_renderer (struct renderer*, struct vox_node*);
void render (struct renderer*, struct vox_node*);

#endif
