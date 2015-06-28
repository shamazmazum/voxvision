#include <SDL/SDL.h>
#include <sys/time.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <iniparser.h>

#include <voxtrees.h>
#include <voxrnd.h>
#include "reader.h"

#define read_vector_or_quit(str, ctrl, x, y, z, errormsg) do {   \
    res = sscanf (str, ctrl, x, y, z); \
    if (res != 3) { \
    fprintf (stderr, errormsg); \
    exit (1); }} while (0);

double gettime ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return (double)tv.tv_sec + (0.000001 * (double)tv.tv_usec);
}

static int collidep (vox_dot newpos, struct vox_node *tree)
{
    return (!(vox_tree_ball_collidep (tree, newpos, 50)));
}

int main (int argc, char *argv[])
{
    printf ("This is my simple renderer version %i.%i\n", VOX_VERSION_MAJOR, VOX_VERSION_MINOR);    
    if (argc != 2)
    {
        fprintf (stderr, "Usage: demo config\n");
        exit (1);
    }

    dimension d;
    vox_dot *set;
    dictionary *cfg = iniparser_load (argv[1]);
    if (cfg == NULL)
    {
        fprintf (stderr, "Cannot load cfg file\n");
        exit(1);
    }

    char path[MAXPATHLEN];
    strcpy (path, argv[1]);
    int path_len = strlen (path);
    char *slash = path+path_len;
    while ((slash!=path) && (*slash != '/')) slash--;
    *slash = '\0';
    if (path[0] != '\0') chdir (path);

    int res;
    read_vector_or_quit (iniparser_getstring (cfg, "Scene:Geometry", ""),
                         "<%i,%i,%i>", &(d.x), &(d.y), &(d.z),
                         "Specify correct geometry\n");

    float x,y,z;
    read_vector_or_quit (iniparser_getstring (cfg, "Scene:Voxsize", "<1,1,1>"),
                         "<%f,%f,%f>", &x, &y, &z,
                         "Specify correct voxsize\n");
    mul[0] = x; mul[1] = y; mul[2] = z;
    vox_voxel[0] = x; vox_voxel[1] = y; vox_voxel[2] = z;

    vox_dot origin;
    read_vector_or_quit (iniparser_getstring (cfg, "Camera:Position", "<0,-100,0>"),
                         "<%f,%f,%f>", &origin[0], &origin[1], &origin[2],
                         "Specify correct camera position\n");
    float fov = (float)iniparser_getdouble (cfg, "Camera:Fov", 1.0);
    vox_simple_camera *camera = vox_make_simple_camera (fov, origin);

    float rotx,roty,rotz;
    read_vector_or_quit (iniparser_getstring (cfg, "Camera:Rot", "<0,0,0>"),
                         "<%f,%f,%f>", &rotx, &roty, &rotz,
                         "Specify correct rotation angles\n");
    camera->iface.set_rot_angles (camera, rotx, roty, rotz);
    
    int fd = open (iniparser_getstring (cfg, "Scene:DataSet", ""), O_RDONLY);
    if (fd == -1)
    {
        fprintf (stderr, "Cannot open dataset\n");
        exit(1);
    }

    int threshold = iniparser_getint (cfg, "Scene:Threshold", 30);
    int samplesize = iniparser_getint (cfg, "Scene:SampleSize", 1);
    printf ("Reading raw data\n");
    int length = read_data (fd, &set, &d, samplesize, threshold);
    close (fd);
    if (length == -1)
    {
        fprintf (stderr, "Cannot read dataset\n");
        exit(1);
    }

    int w = iniparser_getint (cfg, "Window:Width", 800);
    int h = iniparser_getint (cfg, "Window:Height", 600);

    iniparser_freedict (cfg);

    double time = gettime ();
    struct vox_node *tree = vox_make_tree (set, length);
    time = gettime() - time;
    printf ("Building tree (%i voxels, %i depth) took %f\n", vox_voxels_in_tree (tree),
            vox_inacc_depth (tree), time);
    printf ("Tree balanceness %f\n", vox_inacc_balanceness (tree));

    if (SDL_Init (SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Cannot init SDL\n");
        exit (1);
    }

    SDL_EnableUNICODE(SDL_ENABLE);

    SDL_Surface *screen = SDL_SetVideoMode(w, h, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "Cannot init screen\n");
        exit (1);
    }

    SDL_Rect rect;
    rect.w = screen->w; rect.h = screen->h;
    rect.x = 0; rect.y = 0;
    int redraw = 1;
    SDL_Event event;
    do
    {
        if (redraw)
        {
            time = gettime();
            vox_render (tree, &(camera->iface), screen);
            time = gettime() - time;
            printf ("Rendering took %f\n", time);
            redraw = 0;
        }
        if (SDL_WaitEvent(&event))
        {
            switch (event.type) {
            case SDL_KEYDOWN:
            {
                if ((event.key.keysym.unicode == 'a') ||
                    (event.key.keysym.unicode == 'A') ||
                    (event.key.keysym.unicode == 'w') ||
                    (event.key.keysym.unicode == 'W') ||
                    (event.key.keysym.unicode == 's') ||
                    (event.key.keysym.unicode == 'S'))
                {
                    vox_dot step = {0,0,0};
                         if (event.key.keysym.unicode == 'a') step[0] = -5;
                    else if (event.key.keysym.unicode == 'A') step[0] = 5;
                    else if (event.key.keysym.unicode == 'w') step[2] = -5;
                    else if (event.key.keysym.unicode == 'W') step[2] = 5;
                    else if (event.key.keysym.unicode == 's') step[1] = -5;
                    else if (event.key.keysym.unicode == 'S') step[1] = 5;
                    camera->iface.move_camera (camera, step, collidep, tree);
                    redraw = 1;
                }
                else if ((event.key.keysym.unicode == 'x') ||
                         (event.key.keysym.unicode == 'X') ||
                         (event.key.keysym.unicode == 'z') ||
                         (event.key.keysym.unicode == 'Z'))
                {
                    float rotx, roty, rotz;
                    camera->iface.get_rot_angles (camera, &rotx, &roty, &rotz);
                         if (event.key.keysym.unicode == 'x') rotx += 0.01;
                    else if (event.key.keysym.unicode == 'X') rotx -= 0.01;
                    else if (event.key.keysym.unicode == 'z') rotz += 0.01;
                    else if (event.key.keysym.unicode == 'Z') rotz -= 0.01;
                    camera->iface.set_rot_angles (camera, rotx, roty, rotz);
                    redraw = 1;
                }
                SDL_FillRect (screen, &rect, SDL_MapRGB (screen->format, 0,0,0));
                break;
            }
            case SDL_QUIT:
                SDL_Quit();
                vox_destroy_tree (tree);
                free (set);
                exit(0);
            }
        }
    } while (1);
    return 0;
}
