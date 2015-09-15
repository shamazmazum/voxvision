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

/* FIXME: There is a standard POSIX way for this? */
int get_file_directory (const char *path, char *dir)
{
    int res, len;
    char *cursor;
    strcpy (dir, path);
    len = strlen (path);
    cursor = dir + len - 1;
    while (*cursor != '/')
    {
        if (cursor == dir) return 0;
        cursor--;
    }
    *cursor = '\0';
    return 1;
}

int main (int argc, char *argv[])
{
    dimension d;
    vox_dot *set;
    dictionary *cfg = NULL;
    int fd = -1, cwd = -1, res = 0;
    double time;

    struct vox_node *tree = NULL;
    vox_simple_camera *camera = NULL;
    struct vox_rnd_ctx *ctx = NULL;

    printf ("This is my simple renderer version %i.%i\n", VOX_VERSION_MAJOR, VOX_VERSION_MINOR);    
    if (argc != 2)
    {
        fprintf (stderr, "Usage: voxvision-demo config\n");
        goto end;
    }

    cfg = iniparser_load (argv[1]);
    if (cfg == NULL)
    {
        fprintf (stderr, "Cannot load cfg file\n");
        goto end;
    }

    cwd = open (".", O_RDONLY);
    char path[MAXPATHLEN];
    if (get_file_directory (argv[1], path)) chdir (path);

    read_vector_or_quit (iniparser_getstring (cfg, "Scene:Geometry", ""),
                         "<%i,%i,%i>", &(d.x), &(d.y), &(d.z),
                         "Specify correct geometry\n");

    float x,y,z;
    read_vector_or_quit (iniparser_getstring (cfg, "Scene:Voxsize", "<1,1,1>"),
                         "<%f,%f,%f>", &x, &y, &z,
                         "Specify correct voxsize\n");
    mul[0] = x; mul[1] = y; mul[2] = z;
    vox_voxel[0] = x; vox_voxel[1] = y; vox_voxel[2] = z;

    fd = open (iniparser_getstring (cfg, "Scene:DataSet", ""), O_RDONLY);
    if (fd == -1)
    {
        fprintf (stderr, "Cannot open dataset\n");
        goto end;
    }

    int threshold = iniparser_getint (cfg, "Scene:Threshold", 30);
    int samplesize = iniparser_getint (cfg, "Scene:SampleSize", 1);
    printf ("Reading raw data\n");
    int length = read_data (fd, &set, &d, samplesize, threshold);

    if (length == -1)
    {
        fprintf (stderr, "Cannot read dataset\n");
        goto end;
    }
    close (fd);
    fd = -1;
    fchdir (cwd);
    close (cwd);
    cwd = -1;

    time = gettime ();
    tree = vox_make_tree (set, length);
    time = gettime() - time;
    printf ("Building tree (%i voxels, %i depth) took %f\n", vox_voxels_in_tree (tree),
            vox_inacc_depth (tree), time);
    printf ("Tree balanceness %f\n", vox_inacc_balanceness (tree));

    vox_dot origin;
    read_vector_or_quit (iniparser_getstring (cfg, "Camera:Position", "<0,-100,0>"),
                         "<%f,%f,%f>", &origin[0], &origin[1], &origin[2],
                         "Specify correct camera position\n");
    float fov = (float)iniparser_getdouble (cfg, "Camera:Fov", 1.0);
    camera = vox_make_simple_camera (fov, origin);

    vox_dot angles;
    read_vector_or_quit (iniparser_getstring (cfg, "Camera:Rot", "<0,0,0>"),
                         "<%f,%f,%f>", &(angles[0]), &(angles[1]), &(angles[2]),
                         "Specify correct rotation angles\n");
    camera->iface.set_rot_angles (camera, angles);

    int w = iniparser_getint (cfg, "Window:Width", 800);
    int h = iniparser_getint (cfg, "Window:Height", 600);

    iniparser_freedict (cfg);
    cfg = NULL;

    res = 1;
    if (SDL_Init (SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Cannot init SDL\n");
        res = 0;
        goto end;
    }

    SDL_Surface *screen = SDL_SetVideoMode(w, h, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "Cannot init screen\n");
        goto end;
    }

    ctx = vox_make_renderer_context (screen, tree, &(camera->iface));

    SDL_Rect rect;
    rect.w = screen->w; rect.h = screen->h;
    rect.x = 0; rect.y = 0;

    printf ("Controls: WASD,1,2 - movement. Arrows - camera rotation\n");
    printf ("Other keys: q - quit. F11 - take screenshot in screen.bmp in "
            "the current directory\n");

    int count = 0;
    time = gettime ();
    while (1)
    {
        SDL_Event event;
        SDL_FillRect (screen, &rect, SDL_MapRGB (screen->format, 0, 0, 0));
        vox_render (ctx);
        vox_dot step = {0,0,0};
        vox_dot rot_delta = {0,0,0};
        Uint8 *keystate = SDL_GetKeyState (NULL);
        if (keystate[SDLK_w]) step[1] += 5;
        else if (keystate[SDLK_s]) step[1] -= 5;
        if (keystate[SDLK_d]) step[0] += 5;
        else if (keystate[SDLK_a]) step[0] -= 5;
        if (keystate[SDLK_1]) step[2] += 5;
        else if (keystate[SDLK_2]) step[2] -= 5;
        if (keystate[SDLK_UP]) rot_delta[0] -= 0.01;
        else if (keystate[SDLK_DOWN]) rot_delta[0] += 0.01;
        if (keystate[SDLK_LEFT]) rot_delta[2] += 0.01;
        else if (keystate[SDLK_RIGHT]) rot_delta[2] -= 0.01;
        camera->iface.rotate_camera (camera, rot_delta);
        camera->iface.move_camera (camera, step);

        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case 'q':
                    goto end;
                case SDLK_F11:
                    SDL_SaveBMP (screen, "screen.bmp");
                    break;
                    /* Just to silence clang warning */
                default: ;
                }
                break;
            case SDL_QUIT:
                goto end;
            }
        }

        count++;
        double time2 = gettime() - time;
        if (time2 >= 1.0)
        {
            printf ("%i frames in %f seconds\n", count, time2);
            time = gettime();
            count = 0;
        }
    }

end:
    if (cfg != NULL) iniparser_freedict (cfg);
    if (cwd >= 0) close (cwd);
    if (fd  >= 0) close (fd);
    if (ctx != NULL) free (ctx);
    if (camera != NULL) free (camera);
    if (tree != NULL)
    {
        vox_destroy_tree (tree);
        free (set);
    }
    if (res) SDL_Quit();

    return 0;
}
