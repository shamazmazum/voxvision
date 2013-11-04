// Simple slow renderer
#include <SDL/SDL.h>
#include <stdio.h>
#include <assert.h>

#include "../voxtrees/voxtrees.h"
#include "renderer.h"

static void color_coeff (struct renderer *rnd, struct vox_node *tree)
{
    int i;
    for (i=0; i<3; i++)
    {
        rnd->cmul[i] = 255 / (tree->bb_max[i] - tree->bb_min[i]);
        rnd->cadd[i] = -255 * tree->bb_min[i] / (tree->bb_max[i] - tree->bb_min[i]);
    }
}

static void draw_pixel (struct renderer *rnd, vox_dot inter, int p)
{
    Uint8 r = rnd->cmul[0]*inter[0]+rnd->cadd[0];
    Uint8 g = rnd->cmul[1]*inter[1]+rnd->cadd[1];
    Uint8 b = rnd->cmul[2]*inter[2]+rnd->cadd[2];
    Uint32 color = SDL_MapRGB (rnd->surf->format, r, g, b);
    *((Uint32*)rnd->surf->pixels+p) = color;
}

static void render_line (struct renderer *rnd, struct vox_node *tree)
{
    int i,j;
    int n, state;
    int path_idx = 1;
    vox_tree_path path;
    vox_dot inter;

    n = vox_ray_tree_intersection (tree, rnd->origin, rnd->dir, inter, 1, path);
    state = n;
    if (state) draw_pixel (rnd, inter, rnd->p);
    rnd->dir[0] += rnd->dx;

    for (i=1; i<4; i++)
    {
        if (state)
        {
            path_idx = vox_local_rays_tree_intersection (path, rnd->origin, rnd->dir, inter, path_idx, n);
            state = path_idx;
        }
        if (!(state)) path_idx = vox_ray_tree_intersection (tree, rnd->origin, rnd->dir, inter, 1, path);
        
        // End loop
        if (path_idx) draw_pixel (rnd, inter, rnd->p+i);
        rnd->dir[0] += rnd->dx;
    }
    rnd->p+=4;
}

void init_renderer (struct renderer *rnd, struct vox_node *tree)
{
    color_coeff (rnd, tree);
    rnd->dir[1] = 400;

    rnd->dx = 2*400.0*rnd->fov/rnd->surf->w;
    rnd->dy = 2*400.0*rnd->fov/rnd->surf->h;
}

void render (struct renderer *rnd, struct vox_node *tree)
{
    assert (rnd->surf->w%4 == 0);
    rnd->p = 0;
    
    int i,j;
    rnd->dir[2] = -400*rnd->fov;
    for (i=0; i<rnd->surf->h; i++)
    {
        rnd->dir[0] = -400*rnd->fov;
        for (j=0; j<rnd->surf->w; j+=4) render_line (rnd, tree);
        rnd->dir[2] += rnd->dy;
    }

    SDL_Flip (rnd->surf);
}
