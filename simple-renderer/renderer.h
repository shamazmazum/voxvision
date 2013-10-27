#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <SDL/SDL.h>
#include "../voxtrees/tree.h"

void render (struct vox_node*, SDL_Surface*, const vox_dot, float);

#endif
