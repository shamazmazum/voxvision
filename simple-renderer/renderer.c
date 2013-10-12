// Simple slow renderer
#include <SDL/SDL.h>
#include <stdio.h>

#include "../lib/voxvision.h"
#include "renderer.h"

void color_coeff (struct node *tree, float *mul, float *add)
{
    int i;
    for (i=0; i<3; i++)
    {
        mul[i] = 255 / (tree->bb_max[i] - tree->bb_min[i]);
        add[i] = -255 * tree->bb_min[i] / (tree->bb_max[i] - tree->bb_min[i]);
    }
}

void render (struct node *tree, SDL_Surface *screen, const float *origin, float fov, int lod)
{
    float color_mul[3];
    float color_add[3];
    color_coeff (tree, color_mul, color_add);

    int sx,sy;
    float dir[3];
    float d = 400.0;
    dir[1] = d;

    int interp;
    float inter[3];
    int p = 0;

    for (sy=0; sy<screen->h; sy++)
    {
        for (sx=0; sx<screen->w; sx++)
        {
            dir[0] = d * (-fov + (2*sx*fov)/screen->w);
            dir[2] = d * (-fov + (2*sy*fov)/screen->h);

            interp = ray_tree_intersection (tree, origin, dir, inter, 1, lod);
            if (interp)
            {
                Uint8 r = color_mul[0]*inter[0]+color_add[0];
                Uint8 g = color_mul[1]*inter[1]+color_add[1];
                Uint8 b = color_mul[2]*inter[2]+color_add[2];
                Uint32 color = SDL_MapRGB (screen->format, r, g, b);
                *((Uint32*)screen->pixels+p)= color;
            }
            p++;
        }
    }
    SDL_Flip (screen);
}
