#include <SDL/SDL.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "renderer.h"
#include "data.h"
#include "../lib/tree.h"

double gettime ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return (double)tv.tv_sec + (0.000001 * (double)tv.tv_usec);
}

int main (int argc, char *argv[])
{
    if (argc != 3)
    {
        printf ("Usage: test_renderer tree|skull LOD\n");
        exit (1);
    }
    
    int lod = atoi (argv[2]);
    float (*set)[N];
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
    struct node *tree = make_tree (set, length);
    time = gettime() - time;

    printf ("Building tree (%i voxels, %i depth) took %f\n", voxels_in_tree (tree), inacc_depth (tree, 0), time);
    printf ("Tree balanceness %f\n", inacc_balanceness (tree));

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

    time = gettime();
    render (tree, screen, 1.2, lod);
    time = gettime() - time;

    printf ("Rendering took %f\n", time);

    while (1)
    {
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            switch (event.type) {
            case SDL_QUIT:
                SDL_Quit();
                exit(0);
            }
        }
    }
    return 0;
}
