#include <SDL/SDL.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "../voxtrees/voxtrees.h"
#include "../voxrnd/voxrnd.h"
#include "reader.h"

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
    if (argc != 4)
    {
        fprintf (stderr, "Usage: test_renderer dataset geometry multiplier_geometry\n");
        exit (1);
    }

    dimension d;
    vox_dot *set;
    int res = sscanf (argv[2], "%ix%ix%i", &(d.x), &(d.y), &(d.z));
    if (res != 3)
    {
        fprintf (stderr, "Specify correct geometry\n");
        exit(1);
    }

    float x,y,z;
    res = sscanf (argv[3], "%fx%fx%f", &x, &y, &z);
    if (res != 3)
    {
        fprintf (stderr, "Specify correct geometry\n");
        exit(1);
    }
    mul[0] = x; mul[1] = y; mul[2] = z;
    vox_voxel[0] = x; vox_voxel[1] = y; vox_voxel[2] = z;

    int fd = open (argv[1], O_RDONLY);
    if (fd == -1)
    {
        fprintf (stderr, "Cannot open dataset\n");
        exit(1);
    }

    printf ("Reading raw data\n");
    int length = read_data (fd, &set, &d, 1, 27);
    close (fd);
    if (length == -1)
    {
        fprintf (stderr, "Cannot read dataset\n");
        exit(1);
    }

    double time = gettime ();
    struct vox_node *tree = vox_make_tree (set, length);
    time = gettime() - time;
    printf ("Building tree (%i voxels, %i depth) took %f\n", vox_voxels_in_tree (tree), vox_inacc_depth (tree, 0), time);
    printf ("Tree balanceness %f\n", vox_inacc_balanceness (tree));

    if (SDL_Init (SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Cannot init SDL\n");
        exit (1);
    }

    SDL_EnableUNICODE(SDL_ENABLE);

    SDL_Surface *screen = SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "Cannot init screen\n");
        exit (1);
    }

    vox_dot origin = {50,-100,50};
    vox_camera camera;
    vox_make_simple_camera (&camera, 1.2, origin);
    
    time = gettime();
    vox_render (tree, &camera, screen);
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
                    float *pos = camera.position;
                    if (event.key.keysym.unicode == 'a') origin_inc_test (tree, pos, 0, -5.0);
                    else if (event.key.keysym.unicode == 'A') origin_inc_test (tree, pos, 0, 5.0);
                    else if (event.key.keysym.unicode == 'w') origin_inc_test (tree, pos, 2, -5.0);
                    else if (event.key.keysym.unicode == 'W') origin_inc_test (tree, pos, 2, 5.0);
                    else if (event.key.keysym.unicode == 's') origin_inc_test (tree, pos, 1, -5.0);
                    else if (event.key.keysym.unicode == 'S') origin_inc_test (tree, pos, 1, 5.0);
                    else if (event.key.keysym.unicode == 'x') camera.rotx += 0.01;
                    else if (event.key.keysym.unicode == 'X') camera.rotx -= 0.01;
                    else if (event.key.keysym.unicode == 'z') camera.rotz += 0.01;
                    else if (event.key.keysym.unicode == 'Z') camera.rotz -= 0.01;

                    camera.update_rotation (&camera);
                    SDL_Rect rect = {0,0,800,600};
                    SDL_FillRect (screen, &rect, SDL_MapRGB (screen->format, 0,0,0));
                    vox_render (tree, &camera, screen);
                    time = gettime() - time;
                    printf ("Rendering took %f\n", time);
                    printf ("Camera position: %f %f %f\n", pos[0], pos[1], pos[2]);
                    printf ("Rotations: around Ox = %f, around Oz = %f\n\n", camera.rotx, camera.rotz);
                break;
            case SDL_QUIT:
                SDL_Quit();
                vox_destroy_tree (tree);
                free (set);
                exit(0);
            }
        }
    }
    return 0;
}
