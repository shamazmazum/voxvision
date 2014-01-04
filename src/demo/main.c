#include <SDL/SDL.h>
#include <sys/time.h>

#include "data.h"
#include "../voxtrees/voxtrees.h"
#include "../voxrnd/voxrnd.h"

double gettime ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return (double)tv.tv_sec + (0.000001 * (double)tv.tv_usec);
}

static void origin_inc_test (struct vox_node *tree, float *pos, int idx, float val)
{
    pos[idx] += val;
    if (vox_tree_ball_collidep (tree, pos, 50)) pos[idx] -= val;
}

int main (int argc, char *argv[])
{
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

    SDL_EnableUNICODE(SDL_ENABLE);

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
                    time = gettime();
                    float *pos = vox_camera_position_ptr (cam);
                    if (event.key.keysym.unicode == 'a') origin_inc_test (tree, pos, 0, -5.0);
                    else if (event.key.keysym.unicode == 'A') origin_inc_test (tree, pos, 0, 5.0);
                    else if (event.key.keysym.unicode == 'w') origin_inc_test (tree, pos, 2, -5.0);
                    else if (event.key.keysym.unicode == 'W') origin_inc_test (tree, pos, 2, 5.0);
                    else if (event.key.keysym.unicode == 's') origin_inc_test (tree, pos, 1, -5.0);
                    else if (event.key.keysym.unicode == 'S') origin_inc_test (tree, pos, 1, 5.0);
                    else if (event.key.keysym.unicode == 'x') SETTER_NAME(rotx) (cam, GETTER_NAME(rotx) (cam) + 0.01);
                    else if (event.key.keysym.unicode == 'X') SETTER_NAME(rotx) (cam, GETTER_NAME(rotx) (cam) - 0.01);
                    else if (event.key.keysym.unicode == 'z') SETTER_NAME(rotz) (cam, GETTER_NAME(rotz) (cam) + 0.01);
                    else if (event.key.keysym.unicode == 'Z') SETTER_NAME(rotz) (cam, GETTER_NAME(rotz) (cam) - 0.01);

                    SDL_Rect rect = {0,0,800,600};
                    SDL_FillRect (screen, &rect, SDL_MapRGB (screen->format, 0,0,0));
                    vox_render (tree, ctx);
                    time = gettime() - time;
                    printf ("Rendering took %f\n", time);
                    printf ("Camera position: %f %f %f\n", pos[0], pos[1], pos[2]);
                    printf ("Rotations: around Ox = %f, around Oz = %f\n\n",
                            GETTER_NAME(rotx) (cam), GETTER_NAME(rotz) (cam));
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
