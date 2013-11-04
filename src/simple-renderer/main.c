#include <SDL/SDL.h>
#include <sys/time.h>
#include <string.h>
#if 0
#include <gc.h>
#else
#include <stdlib.h>
#endif

#include "renderer.h"
#include "data.h"
#include "../voxtrees/voxtrees.h"

double gettime ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return (double)tv.tv_sec + (0.000001 * (double)tv.tv_usec);
}

static void origin_inc_test (struct vox_node *tree, vox_dot origin, int idx, float val)
{
    origin[idx] += val;
    if (vox_tree_ball_collidep (tree, origin, 50)) origin[idx] -= val;
}

int main (int argc, char *argv[])
{
#if 0
    GC_INIT();
#endif

    printf ("This is my simple renderer version %i.%i\n", VOX_VERSION_MAJOR, VOX_VERSION_MINOR);    
    if (argc != 2)
    {
        printf ("Usage: test_renderer tree|skull\n");
        exit (1);
    }
    
    vox_dot *set;
    int length;
    if (strcmp (argv[1], "tree") == 0)
    {
        set = xmas_tree;
        length = 935859;
    }
    else if (strcmp (argv[1], "skull") == 0)
    {
        set = skull;
        length = 1194419;
    }
    else
    {
        printf ("Invalid data set\n");
        exit(1);
    }
    
    double time = gettime ();
    struct vox_node *tree = vox_make_tree (set, length);
    time = gettime() - time;

    printf ("Building tree (%i voxels, %i depth) took %f\n", vox_voxels_in_tree (tree), vox_inacc_depth (tree, 0), time);
    printf ("Tree balanceness %f\n", vox_inacc_balanceness (tree));

    if (SDL_Init (SDL_INIT_VIDEO) != 0)
    {
        printf("Cannot init SDL\n");
        exit (1);
    }

    SDL_Surface *screen = SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        printf("Cannot init screen\n");
        exit (1);
    }

    struct renderer rnd;
    rnd.fov = 1.2;
    rnd.origin[0] = 0;
    rnd.origin[1] = 0;
    rnd.origin[2] = 0;
    rnd.surf = screen;
    init_renderer (&rnd, tree);
    
    time = gettime();
    render (&rnd, tree);
    time = gettime() - time;

    printf ("Rendering took %f\n", time);

    while (1)
    {
        SDL_Event event;
        if (SDL_WaitEvent(&event))
        {
            switch (event.type) {
                case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_a) origin_inc_test (tree, rnd.origin, 0, -5.0);
                else if (event.key.keysym.sym == SDLK_d) origin_inc_test (tree, rnd.origin, 0, 5.0);
                else if (event.key.keysym.sym == SDLK_s) origin_inc_test (tree, rnd.origin, 2, -5.0);
                else if (event.key.keysym.sym == SDLK_w) origin_inc_test (tree, rnd.origin, 2, 5.0);
                else if (event.key.keysym.sym == SDLK_DOWN) origin_inc_test (tree, rnd.origin, 1, -5.0);
                else if (event.key.keysym.sym == SDLK_UP) origin_inc_test (tree, rnd.origin, 1, 5.0);
                SDL_Rect rect = {0,0,800,600};
                SDL_FillRect (screen, &rect, SDL_MapRGB (screen->format, 0,0,0));
                render (&rnd, tree);
                break;
            case SDL_QUIT:
                SDL_Quit();
                exit(0);
            }
        }
    }
    return 0;
}
