#include <sys/time.h>
#include <sys/param.h>

#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL/SDL.h>
#include <iniparser.h>
#ifdef USE_GCD
#include <dispatch/dispatch.h>
#endif

#include <voxtrees.h>
#include <voxrnd.h>
#include "reader.h"
#include "config.h"

#define read_vector_or_quit(str, ctrl, x, y, z, errormsg) do {   \
    int res = sscanf (str, ctrl, x, y, z); \
    if (res != 3) { \
    fprintf (stderr, errormsg); \
    exit (1); }} while (0);

static double gettime ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return (double)tv.tv_sec + (0.000001 * (double)tv.tv_usec);
}

/* FIXME: There is a standard POSIX way for this? */
static int get_file_directory (const char *path, char *dir)
{
    int res, len;
    char *cursor;
    strcpy (dir, path);
    len = strlen (path);
    cursor = dir + len;
    while (*cursor != '/')
    {
        if (cursor == dir) return 0;
        cursor--;
    }
    *(cursor+1) = '\0';
    return 1;
}

static void usage ()
{
    fprintf (stderr, "Usage: voxvision-demo [-c config-file] dataset-config\n");
    exit (1);
}

static void amend_box (struct vox_node **tree, vox_dot center, int size, int add)
{
    int i,j,k;
    vox_dot dot;
    for (i=-size; i<size; i++)
    {
        for (j=-size; j<size; j++)
        {
            for (k=-size; k<size; k++)
            {
                dot[0] = center[0] + i*vox_voxel[0];
                dot[1] = center[1] + j*vox_voxel[1];
                dot[2] = center[2] + k*vox_voxel[2];
                if (add) vox_insert_voxel (tree, dot);
                else vox_delete_voxel (tree, dot);
            }
        }
    }
}

int main (int argc, char *argv[])
{
    dimension d;
    vox_dot *set;
    dictionary *cfg;
    int fd = -1, sdl_init = 0, ch;
    double time;

    vox_simple_camera *camera = NULL;
    struct vox_rnd_ctx *ctx = NULL;
#ifdef USE_GCD
    __block struct vox_node *tree = NULL;
    dispatch_queue_t tree_queue = NULL;
    dispatch_group_t tree_group = NULL;
#else
    struct vox_node *tree = NULL;
#endif

    printf ("This is my simple renderer version %i.%i\n", VOX_VERSION_MAJOR, VOX_VERSION_MINOR);

    // Parse command line arguments
    while ((ch = getopt (argc, argv, "c:")) != -1)
    {
        switch (ch)
        {
        case 'c':
            if (load_configuration (optarg)) fprintf (stderr, "Cannot load configuration file\n");
            break;
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1) usage();
    const char *datacfgname = argv[0];

    // Parse dataset configuration file
    cfg = iniparser_load (datacfgname);
    if (cfg == NULL)
    {
        fprintf (stderr, "Cannot load scene configuration file\n");
        return 1;
    }

    read_vector_or_quit (iniparser_getstring (cfg, "Scene:Geometry", ""),
                         "<%i,%i,%i>", &(d.x), &(d.y), &(d.z),
                         "Specify correct geometry\n");

    float x,y,z;
    read_vector_or_quit (iniparser_getstring (cfg, "Scene:Voxsize", "<1,1,1>"),
                         "<%f,%f,%f>", &x, &y, &z,
                         "Specify correct voxsize\n");

    const char *datasetname = iniparser_getstring (cfg, "Scene:DataSet", "");
    int threshold = iniparser_getint (cfg, "Scene:Threshold", 30);
    int samplesize = iniparser_getint (cfg, "Scene:SampleSize", 1);

    vox_dot origin;
    read_vector_or_quit (iniparser_getstring (cfg, "Camera:Position", "<0,-100,0>"),
                         "<%f,%f,%f>", &origin[0], &origin[1], &origin[2],
                         "Specify correct camera position\n");
    float fov = (float)iniparser_getdouble (cfg, "Camera:Fov", 1.0);


    vox_dot angles;
    read_vector_or_quit (iniparser_getstring (cfg, "Camera:Rot", "<0,0,0>"),
                         "<%f,%f,%f>", &(angles[0]), &(angles[1]), &(angles[2]),
                         "Specify correct rotation angles\n");
    iniparser_freedict (cfg);


    // Read dataset
    char dataset_path[MAXPATHLEN];
    if (!get_file_directory (datacfgname, dataset_path))
        strcpy (dataset_path, "./");
    strcat (dataset_path, datasetname);

    fd = open (dataset_path, O_RDONLY);
    if (fd == -1)
    {
        fprintf (stderr, "Cannot open dataset\n");
        goto end;
    }

    printf ("Reading raw data\n");
    mul[0] = x; mul[1] = y; mul[2] = z;
    vox_voxel[0] = x; vox_voxel[1] = y; vox_voxel[2] = z;
    int length = read_data (fd, &set, &d, samplesize, threshold);
    if (length == -1)
    {
        fprintf (stderr, "Cannot read dataset\n");
        goto end;
    }

    close (fd);
    fd = -1;

    // Build voxel tree
    time = gettime ();
    tree = vox_make_tree (set, length);
    time = gettime() - time;
    printf ("Building tree (%lu voxels) took %f\n",
            vox_voxels_in_tree (tree), time);
    free (set);

#if USE_GCD
    // Synchronous tree operations queue and group
    tree_queue = dispatch_queue_create ("tree ops", 0);
    if (tree_queue == NULL) goto end;
    tree_group = dispatch_group_create ();
#endif

    sdl_init = 1;
    if (SDL_Init (SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Cannot init SDL\n");
        sdl_init = 0;
        goto end;
    }

    SDL_Surface *screen = SDL_SetVideoMode(global_settings.window_width,
                                           global_settings.window_height,
                                           32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "Cannot init screen\n");
        goto end;
    }

    camera = vox_make_simple_camera (fov, origin);
    camera->iface->set_rot_angles (camera, angles);
    ctx = vox_make_renderer_context (screen, tree, camera->iface);

    SDL_Rect rect;
    rect.w = screen->w; rect.h = screen->h;
    rect.x = 0; rect.y = 0;

    printf ("Default controls: WASD,1,2 - movement. Arrows,z,x - camera rotation\n");
    printf ("Other keys: q - quit. F11 - take screenshot in screen.bmp in "
            "the current directory\n");

    int count = 0;
    time = gettime ();
    while (1)
    {
        SDL_Event event;
        SDL_FillRect (screen, &rect, SDL_MapRGB (screen->format, 0, 0, 0));
#if USE_GCD
        /*
          Wait for all synchronous tree operations to complete then
          render a frame and again wait for renderer to complete.

          Asynchronous non-destructive tree operations like rebuilding
          a tree still may be executed in parallel on other queues.
        */
        dispatch_sync (tree_queue, ^{vox_render (ctx);});
#else
        vox_render (ctx);
#endif
        vox_dot step = {0,0,0};
        vox_dot rot_delta = {0,0,0};
        Uint8 *keystate = SDL_GetKeyState (NULL);
        float radius;
        if (keystate[global_controls.walk_forwards]) step[1] += 5;
        else if (keystate[global_controls.walk_backwards]) step[1] -= 5;
        if (keystate[global_controls.walk_right]) step[0] += 5;
        else if (keystate[global_controls.walk_left]) step[0] -= 5;
        if (keystate[global_controls.fly_up]) step[2] += 5;
        else if (keystate[global_controls.fly_down]) step[2] -= 5;
        if (keystate[global_controls.look_up]) rot_delta[0] -= 0.01;
        else if (keystate[global_controls.look_down]) rot_delta[0] += 0.01;
        if (keystate[global_controls.tilt_left]) rot_delta[1] += 0.01;
        else if (keystate[global_controls.tilt_right]) rot_delta[1] -= 0.01;
        if (keystate[global_controls.look_left]) rot_delta[2] += 0.01;
        else if (keystate[global_controls.look_right]) rot_delta[2] -= 0.01;
        camera->iface->rotate_camera (camera, rot_delta);
        camera->iface->move_camera (camera, step);

        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if ((event.key.keysym.sym == global_controls.shrink) ||
                    (event.key.keysym.sym == global_controls.grow))
                {
                    radius = vox_simple_camera_get_radius (camera);
                    if (event.key.keysym.sym == global_controls.shrink)
                        radius-=5;
                    else
                        radius+=5;
                    radius = vox_simple_camera_set_radius (camera, radius);
                    printf ("Camera body radius is now %f\n", radius);
                }
                else if ((event.key.keysym.sym == global_controls.insert) ||
                         (event.key.keysym.sym == global_controls.delete))
                {
#ifdef USE_GCD
                    /*
                      Tree modification will be performed when there
                      are no other operations in the group.
                    */
                    dispatch_group_notify (tree_group, tree_queue, ^{
#endif
                    vox_dot inter;
                    vox_dot dir;
                    camera->iface->screen2world (camera, dir, screen->w/2, screen->h/2);
                    const struct vox_node* leaf =
                        vox_ray_tree_intersection (tree, camera->iface->get_position (camera),
                                                   dir, inter);
                    if (leaf != NULL)
                    {
                        if (event.key.keysym.sym == global_controls.insert)
                            amend_box (&tree, inter, 5, 1);
                        else
                            amend_box (&tree, inter, 5, 0);
                        vox_rc_set_scene (ctx, tree);
                    }
#ifdef USE_GCD
                    });
#endif
                }
                else if (event.key.keysym.sym == SDLK_q) goto end;
                else if (event.key.keysym.sym == SDLK_r)
                {
#ifdef USE_GCD
                    /*
                      Rebuild the tree asynchronously and then
                      update the context.
                    */
                    dispatch_group_async (tree_group,
                                          dispatch_get_global_queue
                                          (DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
#endif
                    struct vox_node *new_tree = vox_rebuild_tree (tree);
#ifdef USE_GCD
                    dispatch_sync (tree_queue, ^{
#endif
                    vox_destroy_tree (tree);
                    tree = new_tree;
                    vox_rc_set_scene (ctx, tree);
                    printf ("Tree rebuilt\n");
#ifdef USE_GCD
                    });});
#endif
                }
                else if (event.key.keysym.sym == SDLK_F11)
                    SDL_SaveBMP (screen, "screen.bmp");
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
    if (fd  >= 0) close (fd);
    if (ctx != NULL) free (ctx);
    if (camera != NULL) camera->iface->destroy_camera (camera);
    if (tree != NULL) vox_destroy_tree (tree);
    if (sdl_init) SDL_Quit();
#if USE_GCD
    if (tree_queue != NULL) dispatch_release (tree_queue);
    if (tree_group != NULL) dispatch_release (tree_group);
#endif

    return 0;
}
