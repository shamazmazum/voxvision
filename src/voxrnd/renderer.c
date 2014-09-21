#include "renderer.h"
#include "../voxtrees/search.h"

static void color_coeff (const struct vox_node *tree, float mul[], float add[])
{
    int i;
    vox_dot min;
    vox_dot max;
    vox_bounding_box (tree, min, max);
    
    for (i=0; i<3; i++)
    {
        mul[i] = 255 / (max[i] - min[i]);
        add[i] = -255 * min[i] / (max[i] - min[i]);
    }
}

static Uint32 get_color (SDL_PixelFormat *format, vox_dot inter, float mul[], float add[])
{
    Uint8 r = mul[0]*inter[0]+add[0];
    Uint8 g = mul[1]*inter[1]+add[1];
    Uint8 b = mul[2]*inter[2]+add[2];
    Uint32 color = SDL_MapRGB (format, r, g, b);
    return color;
}

void vox_render (const struct vox_node *tree, vox_camera_interface *cam_iface, SDL_Surface *surface)
{
    int w = surface->w;
    int h = surface->h;
    int i,j,p;
    p=0;
    int interp;

    // FIXME: calculate colors in runtime
    // Only a temporary solution to get a colorful output
    float col_mul[3];
    float col_add[3];
    color_coeff (tree, col_mul, col_add);

    float *origin;
    vox_dot dir;
    vox_dot inter;
    origin = cam_iface->get_position(cam_iface->camera);

    struct vox_search_state *state = vox_make_search_state (tree);

    for (i=0; i<h; i++)
    {
        for (j=0; j<w; j++)
        {
            cam_iface->screen2world (cam_iface->camera, dir, w, h, j, i);
            interp = vox_ray_tree_intersection_wstate (state, origin, dir, inter);
            if (interp)
            {
                Uint32 color = get_color (surface->format, inter, col_mul, col_add);
                *((Uint32*)surface->pixels+p) = color;
    
            }
            p++;
        }
    }
    
    SDL_Flip (surface);
}
