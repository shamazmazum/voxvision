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
#include "config.h"

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
    dictionary *cfg = NULL;
    int ch, res;

    struct vox_camera *camera = NULL;
    struct vox_rnd_ctx *ctx = NULL;
    struct vox_light_manager *light_manager = NULL;
    __block struct vox_node *tree = NULL;
    dispatch_queue_t tree_queue = NULL;
    dispatch_group_t tree_group = NULL;
    vox_fps_controller_t fps_controller = NULL;
    struct vox_cd *cd = NULL;
    char shot_name[MAXPATHLEN];
    char dataset_path[MAXPATHLEN];
    char dataset_name[MAXPATHLEN];

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
      Although this is not dangerous to have negative dimensions (read_raw_data()
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


    float light_radius = (float)iniparser_getdouble (cfg, "Light:Radius", 100);
    iniparser_freedict (cfg);
    cfg = NULL;

    // Read dataset
    vox_find_data_file (dataset_name, dataset_path);
    printf ("Reading raw data\n");
    const char *errorstr;
    tree = vox_read_raw_data (dataset_path, (unsigned int*)dim, samplesize,
                              ^(unsigned int sample){return sample > threshold;}, &errorstr);
    if (tree == NULL)
    {
        fprintf (stderr, "Cannot read dataset: %s\n", errorstr);
        goto end;
    }

    // Init SDL
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf (stderr, "Cannot init SDL: %s\n", SDL_GetError());
        goto end;
    }

#if USE_GCD
    // Synchronous tree operations queue and group
    tree_queue = dispatch_queue_create ("tree ops", 0);
    if (tree_queue == NULL) goto end;
    tree_group = dispatch_group_create ();
#endif

    camera = vox_camera_methods ("simple-camera")->construct_camera (NULL);
    camera->iface->set_position (camera, origin);
    camera->iface->set_fov (camera, fov);
    camera->iface->set_rot_angles (camera, angles);
    int width = global_settings.window_width;
    int height = global_settings.window_height;
    ctx = vox_make_context_and_window (width, height);
    if (ctx == NULL)
    {
        fprintf (stderr, "Cannot create the context: %s\n", SDL_GetError());
        goto end;
    }
    vox_context_set_scene (ctx, tree);
    vox_context_set_camera (ctx, camera);
    cd = vox_make_cd();
    vox_cd_attach_camera (cd, camera, 3);
    vox_cd_attach_context (cd, ctx);
    SDL_EventState (SDL_MOUSEMOTION, SDL_DISABLE);
    SDL_SetRelativeMouseMode (SDL_TRUE);

    light_manager = vox_create_light_manager ();
    struct vox_sphere light;
    vox_dot_copy (light.center, origin);
    light.radius = light_radius;
    vox_insert_shadowless_light (light_manager, &light, 0);
    vox_context_set_light_manager (ctx, light_manager);

    printf ("Default controls: WASD,1,2 - movement. Arrows,z,x - camera rotation\n");
    printf ("Other keys: q - quit. F11 - take screenshot in the current directory\n");

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
        vox_redraw (ctx);

        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if ((event.key.keysym.scancode == global_controls.insert) ||
                    (event.key.keysym.scancode == global_controls.delete))
                {
                    /*
                      Tree modification will be performed when there
                      are no other operations in the group.
                    */
                    dispatch_group_notify (tree_group, tree_queue, ^{
                            vox_dot inter, dir, origin;
                            camera->iface->get_position (camera, origin);
                            camera->iface->screen2world (camera, dir, width/2, height/2);
                            const struct vox_node* leaf =
                                vox_ray_tree_intersection (tree, origin, dir, inter);
                            if (leaf != NULL)
                            {
                                if (event.key.keysym.scancode == global_controls.insert)
                                    amend_box (&tree, inter, 5, 1);
                                else
                                    amend_box (&tree, inter, 5, 0);
                                vox_context_set_scene (ctx, tree);
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
                                                      vox_context_set_scene (ctx, tree);
                                                      printf ("Tree rebuilt\n");
                                                  });
                                          });
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_F11)
                {
                    suitable_shot_name (shot_name);
                    SDL_SaveBMP (ctx->surface, shot_name);
                }
                break;
            case SDL_QUIT:
                goto end;
            }
        }

        vox_dot step = {0,0,0};
        vox_dot rot_delta = {0,0,0};
        const Uint8 *keystate = SDL_GetKeyboardState (NULL);
        int x,y;

        if (keystate[global_controls.walk_forwards]) step[1] += 5;
        else if (keystate[global_controls.walk_backwards]) step[1] -= 5;
        if (keystate[global_controls.walk_right]) step[0] += 5;
        else if (keystate[global_controls.walk_left]) step[0] -= 5;
        if (keystate[global_controls.fly_up]) step[2] += 5;
        else if (keystate[global_controls.fly_down]) step[2] -= 5;
        if (keystate[global_controls.tilt_left]) rot_delta[1] += 0.01;
        else if (keystate[global_controls.tilt_right]) rot_delta[1] -= 0.01;
        vox_delete_shadowless_light (light_manager, &light);
        camera->iface->move_camera (camera, step);
        camera->iface->get_position (camera, origin);
        vox_dot_copy (light.center, origin);
        vox_insert_shadowless_light (light_manager, &light, 0);

        vox_cd_collide (cd);
        struct vox_fps_info fps_info = fps_controller();
        if (vox_fpsstatus_updated (fps_info.status))
            printf ("Frames per second: %i\n", vox_fpsstatus_fps (fps_info.status));

        SDL_GetRelativeMouseState (&x, &y);
        rot_delta[2] = -global_settings.xspeed*x;
        rot_delta[0] = global_settings.yspeed*y;
        camera->iface->rotate_camera (camera, rot_delta);
    }

end:
    if (cfg != NULL) iniparser_freedict (cfg);
    if (ctx != NULL) vox_destroy_context (ctx);
    if (camera != NULL) camera->iface->destroy_camera (camera);
    if (light_manager != NULL) vox_destroy_light_manager (light_manager);
    if (tree != NULL) vox_destroy_tree (tree);
    if (SDL_WasInit(0)) SDL_Quit();
#if USE_GCD
    if (tree_queue != NULL) dispatch_release (tree_queue);
    if (tree_group != NULL) dispatch_release (tree_group);
#endif
    if (fps_controller != NULL) vox_destroy_fps_controller (fps_controller);
    if (cd != NULL) free (cd);

    return 0;
}
