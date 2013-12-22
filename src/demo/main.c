#include <SDL/SDL.h>
#include <sys/time.h>
/*#include <string.h>
#if 0
#include <gc.h>
#else
#include <stdlib.h>
#endif*/

#include "data.h"
#include "../voxtrees/voxtrees.h"
#include "../voxrnd/voxrnd.h"

double gettime ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return (double)tv.tv_sec + (0.000001 * (double)tv.tv_usec);
}

static void origin_inc_test (struct vox_node *tree, class_t *camera, int idx, float val)
{
    float *pos = vox_camera_position_ptr (camera);
    pos[idx] += val;
    if (vox_tree_ball_collidep (tree, pos, 50)) pos[idx] -= val;
}

int main (int argc, char *argv[])
{
/*#if 0
    GC_INIT();
#endif
*/

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

    vox_dot origin = {0,0,0};
    vox_simple_camera camera;
    vox_make_simple_camera (&camera, 1.0, origin);
    class_t *cam = (class_t*)&camera;
    vox_rnd_context *ctx = vox_init_renderer_context (screen, cam);
    
    time = gettime();
    vox_render (tree, ctx);
    time = gettime() - time;

    printf ("Rendering took %f\n", time);

    while (1)
    {
        SDL_Event event;
        if (SDL_WaitEvent(&event))
        {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_a) origin_inc_test (tree, cam, 0, -5.0);
                    else if (event.key.keysym.sym == SDLK_d) origin_inc_test (tree, cam, 0, 5.0);
                    else if (event.key.keysym.sym == SDLK_s) origin_inc_test (tree, cam, 2, -5.0);
                    else if (event.key.keysym.sym == SDLK_w) origin_inc_test (tree, cam, 2, 5.0);
                    else if (event.key.keysym.sym == SDLK_DOWN) origin_inc_test (tree, cam, 1, -5.0);
                    else if (event.key.keysym.sym == SDLK_UP) origin_inc_test (tree, cam, 1, 5.0);
                    SDL_Rect rect = {0,0,800,600};
                    SDL_FillRect (screen, &rect, SDL_MapRGB (screen->format, 0,0,0));
                    vox_render (tree, ctx);
                break;
            case SDL_QUIT:
                SDL_Quit();
                vox_destroy_tree (tree);
                vox_free_renderer_context (ctx);
                exit(0);
            }
        }
    }
    return 0;
}
