#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <iniparser.h>
#ifdef USE_GCD
#include <dispatch/dispatch.h>
#else
#include "../gcd-stubs.c"
#endif

#include <voxtrees.h>
#include <voxrnd.h>
#include "reader.h"
#include "config.h"
#include "error.h"

/* FIXME: There is a standard POSIX way for this? */
static int get_file_directory (const char *path, char *dir)
{
    ptrdiff_t len;
    char *cursor;
    len = stpncpy (dir, path, MAXPATHLEN) - dir;
    cursor = dir + len;
    while (*cursor != '/')
    {
        if (cursor == dir) return 0;
        cursor--;
    }
    *(cursor+1) = '\0';
    return 1;
}

static void suitable_shot_name (char *name)
{
    static int i = 0;
    int res;
    struct stat sb;

again:
    do
    {
        sprintf (name, "screen%i.bmp", i);
        res = stat (name, &sb);
        i++;
    } while (res == 0);

    if (errno != ENOENT) goto again;
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
    int dim[3];
    vox_dot *set;
    dictionary *cfg = NULL;
    int fd = -1, ch, res;

    struct vox_camera *camera = NULL;
    struct vox_rnd_ctx *ctx = NULL;
    __block struct vox_node *tree = NULL;
    dispatch_queue_t tree_queue = NULL;
    dispatch_group_t tree_group = NULL;
    SDL_Window *screen = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_Surface *surface = NULL;
    vox_fps_controller_t fps_controller = NULL;
    char shot_name[MAXPATHLEN];
    char dataset_path[MAXPATHLEN];
    char dataset_name[MAXPATHLEN];
    char caption[MAXPATHLEN+20];

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
    /*
      Sanity check dimensions.
      Although this is not dangerous to have negative dimensions (read_data()
      will capture this error anyway, it's probably good idea to inform about
      malformed entry in .cfg file.
    */
    for (res=0; res<3; res++)
    {
        if (dim[res] < 0)
        {
            fprintf (stderr, "You must specify correct geometry\n");
            fprintf (stderr, "geometry element n.%i is invalid: %i\n",
                     res, dim[res]);
            goto end;
        }
    }

    _iniparser_getvector3_float (cfg, "Scene:Voxsize", vox_voxel);

    const char *tmp_set_name = iniparser_getstring (cfg, "Scene:DataSet", NULL);
    if (tmp_set_name == NULL)
    {
        fprintf (stderr, "You must specify dataset\n");
        goto end;
    }
    strncpy (dataset_name, tmp_set_name, MAXPATHLEN);

    int threshold = iniparser_getint (cfg, "Scene:Threshold", 30);
    int samplesize = iniparser_getint (cfg, "Scene:SampleSize", 1);

    // Sanity check on the last two
    if ((samplesize <= 0) || (samplesize > 3))
    {
        fprintf (stderr, "Invalid samplesize: %i. It can be 1, 2 or 3\n", samplesize);
        goto end;
    }
    if ((threshold < 0) || (threshold > (1 << 8*samplesize) - 1))
    {
        fprintf (stderr, "Invalid threshold: %i\n", threshold);
        goto end;
    }

    vox_dot origin = {0,-100,0};
    _iniparser_getvector3_float (cfg, "Camera:Position", origin);
    float fov = (float)iniparser_getdouble (cfg, "Camera:Fov", 1.0);
    if (fov < 0)
    {
        fprintf (stderr, "Field of view cannot be negative: %f", fov);
        goto end;
    }

    vox_dot angles = {0,0,0};
    _iniparser_getvector3_float (cfg, "Camera:Rot", angles);
    iniparser_freedict (cfg);
    cfg = NULL;

    // Read dataset
    if (!get_file_directory (datacfgname, dataset_path))
        strncpy (dataset_path, "./", MAXPATHLEN);
    strncat (dataset_path, dataset_name, MAXPATHLEN);

    fd = open (dataset_path, O_RDONLY);
    if (fd == -1)
    {
        perror ("Cannot open dataset");
        goto end;
    }

    printf ("Reading raw data\n");
    int length = read_data (fd, &set, (unsigned int*)dim, samplesize, threshold);
    if (length == -1)
    {
        fprintf (stderr, "Cannot read dataset: %s\n", get_error_string (gerror));
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
    printf ("Building tree (%zu voxels) took %u ms\n",
            vox_voxels_in_tree (tree), SDL_GetTicks() - time);
    free (set);
    voxtrees_print_statistics ();
    voxtrees_clear_statistics ();

#if USE_GCD
    // Synchronous tree operations queue and group
    tree_queue = dispatch_queue_create ("tree ops", 0);
    if (tree_queue == NULL) goto end;
    tree_group = dispatch_group_create ();
#endif

    if (SDL_CreateWindowAndRenderer (global_settings.window_width,
                                     global_settings.window_height,
                                     0,
                                     &screen, &renderer))
    {
        fprintf (stderr, "Cannot create a window: %s\n",
                 SDL_GetError());
        goto end;
    }
    sprintf (caption, "voxvision-demo: %s", datacfgname);
    SDL_SetWindowTitle (screen, caption);

    texture = SDL_CreateTexture (renderer, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 global_settings.window_width,
                                 global_settings.window_height);
    if (texture == NULL)
    {
        fprintf (stderr, "Cannot create texture: %s\n",
                 SDL_GetError());
        goto end;
    }

    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;
    SDL_PixelFormatEnumToMasks (SDL_PIXELFORMAT_ARGB8888, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
    surface = SDL_CreateRGBSurface (0,
                                    global_settings.window_width,
                                    global_settings.window_height,
                                    bpp, Rmask, Gmask, Bmask, Amask);
    if (surface == NULL)
    {
        fprintf (stderr, "Cannot create surface: %s\n",
                 SDL_GetError());
        goto end;
    }

    camera = vox_simple_camera_iface()->construct_camera (NULL);
    camera->iface->set_position (camera, origin);
    camera->iface->set_fov (camera, fov);
    camera->iface->set_rot_angles (camera, angles);
    int camera_type = 0;
    ctx = vox_make_renderer_context (surface, tree, camera);

    printf ("Default controls: WASD,1,2 - movement. Arrows,z,x - camera rotation\n");
    printf ("Other keys: q - quit. F11 - take screenshot in screen.bmp in "
            "the current directory\n");

    fps_controller = vox_make_fps_controller (global_settings.fps);
    while (1)
    {
        SDL_Event event;
        /*
          Wait for all synchronous tree operations to complete then
          render a frame and again wait for renderer to complete.

          Asynchronous non-destructive tree operations like rebuilding
          a tree still may be executed in parallel on other queues.
        */
        dispatch_sync (tree_queue, ^{vox_render (ctx);});
        SDL_UpdateTexture (texture, NULL, surface->pixels, surface->pitch);
        SDL_FillRect (surface, NULL, 0);
        SDL_RenderCopy (renderer, texture, NULL, NULL);
        SDL_RenderPresent (renderer);

        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if ((event.key.keysym.scancode == global_controls.shrink) ||
                    (event.key.keysym.scancode == global_controls.grow))
                {
                    float radius;
                    radius = vox_simple_camera_get_radius ((struct vox_simple_camera*)camera);
                    if (event.key.keysym.scancode == global_controls.shrink)
                        radius-=5;
                    else
                        radius+=5;
                    radius = vox_simple_camera_set_radius ((struct vox_simple_camera*)camera, radius);
                    printf ("Camera body radius is now %f\n", radius);
                }
                else if ((event.key.keysym.scancode == global_controls.insert) ||
                         (event.key.keysym.scancode == global_controls.delete))
                {
                    /*
                      Tree modification will be performed when there
                      are no other operations in the group.
                    */
                    dispatch_group_notify (tree_group, tree_queue, ^{
                            vox_dot inter, dir, origin;
                            camera->iface->get_position (camera, origin);
                            camera->iface->screen2world (camera, dir, surface->w/2, surface->h/2);
                            const struct vox_node* leaf =
                                vox_ray_tree_intersection (tree, origin, dir, inter);
                            if (leaf != NULL)
                            {
                                if (event.key.keysym.scancode == global_controls.insert)
                                    amend_box (&tree, inter, 5, 1);
                                else
                                    amend_box (&tree, inter, 5, 0);
                                vox_rc_set_scene (ctx, tree);
                            }
                        });
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_Q) goto end;
                else if (event.key.keysym.scancode == SDL_SCANCODE_R)
                {
                    /*
                      Rebuild the tree asynchronously and then
                      update the context.
                    */
                    dispatch_group_async (tree_group,
                                          dispatch_get_global_queue
                                          (DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
                                              struct vox_node *new_tree = vox_rebuild_tree (tree);
                                              dispatch_sync (tree_queue, ^{
                                                      vox_destroy_tree (tree);
                                                      tree = new_tree;
                                                      vox_rc_set_scene (ctx, tree);
                                                      printf ("Tree rebuilt\n");
                                                  });
                                          });
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_F11)
                {
                    suitable_shot_name (shot_name);
                    SDL_SaveBMP (surface, shot_name);
                }
                else if (event.key.keysym.scancode == global_controls.toggle_camera)
                {
                    switch (camera_type)
                    {
                    case 0:
                        camera_type = 1;
                        vox_distorted_camera_iface()->coerce_class (camera);
                        break;
                    case 1:
                        camera_type = 0;
                        vox_simple_camera_iface()->coerce_class (camera);
                        break;
                    }
                }
                break;
            case SDL_QUIT:
                goto end;
            }
        }

        vox_dot step = {0,0,0};
        vox_dot rot_delta = {0,0,0};
        const Uint8 *keystate = SDL_GetKeyboardState (NULL);
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

        struct vox_fps_info fps_info = fps_controller();
        if (vox_fpsstatus_updated (fps_info.status))
            printf ("Frames per second: %i\n", vox_fpsstatus_fps (fps_info.status));
    }

end:
    if (cfg != NULL) iniparser_freedict (cfg);
    if (fd  >= 0) close (fd);
    if (ctx != NULL) free (ctx);
    if (camera != NULL) camera->iface->destroy_camera (camera);
    if (tree != NULL) vox_destroy_tree (tree);
    if (surface != NULL) SDL_FreeSurface(surface);
    if (texture != NULL) SDL_DestroyTexture (texture);
    if (renderer != NULL) SDL_DestroyRenderer (renderer);
    if (screen != NULL) SDL_DestroyWindow (screen);
    if (SDL_WasInit(0)) SDL_Quit();
#if USE_GCD
    if (tree_queue != NULL) dispatch_release (tree_queue);
    if (tree_group != NULL) dispatch_release (tree_group);
#endif
    if (fps_controller != NULL) vox_destroy_fps_controller (fps_controller);
    voxtrees_print_statistics ();

    return 0;
}
