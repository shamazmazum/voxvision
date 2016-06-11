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

static Uint32 timer_event (Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent uevent;

    uevent.type = SDL_USEREVENT;
    uevent.code = 0;
    uevent.data1 = NULL;
    uevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = uevent;

    SDL_PushEvent (&event);
    return interval;
}

int main (int argc, char *argv[])
{
    int dim[3];
    vox_dot *set;
    dictionary *cfg = NULL;
    int fd = -1, ch, res;

    struct vox_camera *camera = NULL;
    struct vox_rnd_ctx *ctx = NULL;
#ifdef USE_GCD
    __block struct vox_node *tree = NULL;
    dispatch_queue_t tree_queue = NULL;
    dispatch_group_t tree_group = NULL;
#else
    struct vox_node *tree = NULL;
#endif
    SDL_TimerID timer_id = 0;
    SDL_Surface *screen = NULL;

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

    res = _iniparser_getvector3_int (cfg, "Scene:Geometry", dim);
    if (!res)
    {
        fprintf (stderr, "You must specify correct geometry\n");
        goto end;
    }

    _iniparser_getvector3_float (cfg, "Scene:Voxsize", vox_voxel);

    const char *datasetname = iniparser_getstring (cfg, "Scene:DataSet", NULL);
    if (datasetname == NULL)
    {
        fprintf (stderr, "You must specify dataset\n");
        goto end;
    }
    int threshold = iniparser_getint (cfg, "Scene:Threshold", 30);
    int samplesize = iniparser_getint (cfg, "Scene:SampleSize", 1);

    vox_dot origin = {0,-100,0};
    _iniparser_getvector3_float (cfg, "Camera:Position", origin);
    float fov = (float)iniparser_getdouble (cfg, "Camera:Fov", 1.0);

    vox_dot angles = {0,0,0};
    _iniparser_getvector3_float (cfg, "Camera:Rot", angles);
    iniparser_freedict (cfg);
    cfg = NULL;

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
    int length = read_data (fd, &set, dim, samplesize, threshold);
    if (length == -1)
    {
        fprintf (stderr, "Cannot read dataset\n");
        goto end;
    }

    close (fd);
    fd = -1;

    // Init SDL
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf (stderr, "Cannot init SDL: %s\n", SDL_GetError());
        goto end;
    }

    // Build voxel tree
    Uint32 time = SDL_GetTicks();
    tree = vox_make_tree (set, length);
    printf ("Building tree (%lu voxels) took %u ms\n",
            vox_voxels_in_tree (tree), SDL_GetTicks() - time);
    free (set);

#if USE_GCD
    // Synchronous tree operations queue and group
    tree_queue = dispatch_queue_create ("tree ops", 0);
    if (tree_queue == NULL) goto end;
    tree_group = dispatch_group_create ();
#endif

    screen = SDL_SetVideoMode(global_settings.window_width,
                              global_settings.window_height,
                              32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf (stderr, "Cannot set video mode: %s\n",
                 SDL_GetError());
        goto end;
    }

    camera = vox_simple_camera_iface()->construct_camera (NULL, fov, origin);
    camera->iface->set_rot_angles (camera, angles);
    ctx = vox_make_renderer_context (screen, tree, camera);

    printf ("Default controls: WASD,1,2 - movement. Arrows,z,x - camera rotation\n");
    printf ("Other keys: q - quit. F11 - take screenshot in screen.bmp in "
            "the current directory\n");

    int count = 0;
#if 0
    timer_id = SDL_AddTimer (1000, timer_event, NULL);
#else
    Uint32 start, end;
    start = SDL_GetTicks();
#endif
    while (1)
    {
        SDL_Event event;
        SDL_FillRect (screen, NULL, 0);
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
        SDL_Flip (screen);

        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if ((event.key.keysym.sym == global_controls.shrink) ||
                    (event.key.keysym.sym == global_controls.grow))
                {
                    float radius;
                    radius = vox_simple_camera_get_radius ((struct vox_simple_camera*)camera);
                    if (event.key.keysym.sym == global_controls.shrink)
                        radius-=5;
                    else
                        radius+=5;
                    radius = vox_simple_camera_set_radius ((struct vox_simple_camera*)camera, radius);
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
            case SDL_USEREVENT:
                printf ("%i fps\n", count);
                count = 0;
                break;
            case SDL_QUIT:
                goto end;
            }
        }

        vox_dot step = {0,0,0};
        vox_dot rot_delta = {0,0,0};
        const Uint8 *keystate = SDL_GetKeyState (NULL);
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

        count++;

        end = SDL_GetTicks();
        Uint32 diff = end - start;
        if (diff > 1000)
        {
            printf ("%i frames in %u ms\n", count, diff);
            start = SDL_GetTicks();
            count = 0;
        }
    }

end:
    if (cfg != NULL) iniparser_freedict (cfg);
    if (fd  >= 0) close (fd);
    if (ctx != NULL) free (ctx);
    if (camera != NULL) camera->iface->destroy_camera (camera);
    if (tree != NULL) vox_destroy_tree (tree);
    if (screen != NULL) SDL_FreeSurface (screen);
    if (timer_id) SDL_RemoveTimer (timer_id);
    if (SDL_WasInit(0)) SDL_Quit();
#if USE_GCD
    if (tree_queue != NULL) dispatch_release (tree_queue);
    if (tree_group != NULL) dispatch_release (tree_group);
#endif

    return 0;
}
