#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#ifdef USE_GCD
#include <dispatch/dispatch.h>
#else
#include "../gcd-stubs.c"
#endif

#include <voxtrees.h>
#include <voxrnd.h>
#include "config.h"

static void usage () {
    fprintf (stderr, "Usage: voxvision-demo [-c config-file] dataset-config\n");
    exit (1);
}

static void amend_box (struct vox_node **tree, vox_dot center, int size, int add) {
    int i,j,k;
    vox_dot dot;
    for (i=-size; i<size; i++) {
        for (j=-size; j<size; j++) {
            for (k=-size; k<size; k++) {
                dot[0] = center[0] + i*vox_voxel[0];
                dot[1] = center[1] + j*vox_voxel[1];
                dot[2] = center[2] + k*vox_voxel[2];
                if (add) {
                    vox_insert_voxel (tree, dot);
                } else {
                    vox_delete_voxel (tree, dot);
                }
            }
        }
    }
}

int main (int argc, char *argv[]) {
    int ch;

    struct vox_camera *camera = NULL;
    struct vox_rnd_ctx *ctx = NULL;
    __block struct vox_node *tree = NULL;
    dispatch_queue_t tree_queue = NULL;
    dispatch_group_t tree_group = NULL;
    vox_fps_controller_t fps_controller = NULL;
    struct vox_cd *cd = NULL;
    struct settings config;

    printf ("This is my simple renderer version %i.%i\n",
            VOX_VERSION_MAJOR, VOX_VERSION_MINOR);

    init_configuration (&config);
    // Parse command line arguments
    while ((ch = getopt (argc, argv, "c:")) != -1) {
        switch (ch) {
        case 'c':
            if (load_configuration (optarg, &config) == 0) {
                fprintf (stderr, "Cannot load configuration file\n");
            }
            break;
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage();
    }

    const char *dataset_name = argv[0];
    vox_dot voxel_size = config.voxel_size;
    if (voxel_size.x < 0 || voxel_size.y < 0 || voxel_size.z < 0) {
        fprintf (stderr, "Wrong voxel size\n");
        goto end;
    }
    vox_voxel = voxel_size;

    double threshold = config.threshold;
    if ((threshold < 0) || (threshold > 1)) {
        fprintf (stderr, "Invalid threshold: %f\n", threshold);
        goto end;
    }

    vox_dot origin = config.camera_pos;
    float fov = config.camera_fov;
    if (fov < 0) {
        fprintf (stderr, "Field of view cannot be negative: %f", fov);
        goto end;
    }

    int width  = config.window_width;
    int height = config.window_height;

    int fps = config.fps;
    if (fps <= 0) {
        fprintf (stderr, "FPS cannot be non-positive: %i", fps);
        goto end;
    }

    float xspeed = config.xspeed;
    if (xspeed <= 0) {
        fprintf (stderr, "Mouse X speed cannot be non-positive: %f", xspeed);
        goto end;
    }

    float yspeed = config.yspeed;
    if (yspeed <= 0) {
        fprintf (stderr, "Mouse Y speed cannot be non-positive: %f", yspeed);
        goto end;
    }

    // Read dataset
    printf ("Reading data\n");
    const char *errorstr;
    tree = vox_read_data (dataset_name,  ^(uint32_t sample, uint8_t bps) {
            bool solid;
            if (bps == 1) {
                solid = sample;
            } else {
                uint32_t th = threshold * (1 << bps);
                solid = sample > th;
            }
            return solid;
        }, &errorstr);

    if (tree == NULL) {
        fprintf (stderr, "Cannot read dataset: %s\n", errorstr);
        goto end;
    }

    // Init SDL
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf (stderr, "Cannot init SDL: %s\n", SDL_GetError());
        goto end;
    }

#if USE_GCD
    // Synchronous tree operations queue and group
    tree_queue = dispatch_queue_create ("tree ops", 0);
    if (tree_queue == NULL) goto end;
    tree_group = dispatch_group_create ();
#endif

    struct vox_camera_interface *camera_iface = vox_camera_methods ("simple-camera");
    if (camera_iface == NULL) {
        fprintf (stderr, "Cannot find camera module\n");
        goto end;
    }

    vox_dot look = (vox_dot) {0, 0, 0, 0};
    camera = camera_iface->construct_camera (NULL);
    camera->iface->set_property_dot (camera, "position", origin);
    camera->iface->set_property_number (camera, "fov", fov);
    camera->iface->look_at (camera, look);

    ctx = vox_make_context_and_window (width, height);
    if (ctx == NULL) {
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

    printf ("Default controls: WASD,1,2 - movement. Arrows,z,x - camera rotation\n");
    printf ("Other keys: q - quit. F11 - take screenshot in the current directory\n");

    unsigned int quality = config.quality;
    vox_context_set_quality (ctx, quality);
    fps_controller = vox_make_fps_controller (fps);
    while (1) {
        SDL_Event event;
        /*
          Wait for all synchronous tree operations to complete then
          render a frame and again wait for renderer to complete.

          Asynchronous non-destructive tree operations like rebuilding
          a tree still may be executed in parallel on other queues.
        */
        dispatch_sync (tree_queue, ^{vox_render (ctx);});
        vox_redraw (ctx);

        if (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if ((event.key.keysym.scancode == config.insert) ||
                    (event.key.keysym.scancode == config.delete)) {
                    /*
                      Tree modification will be performed when there
                      are no other operations in the group.
                    */
                    dispatch_group_notify (tree_group, tree_queue, ^{
                            vox_dot inter, dir, origin;
                            origin = camera->iface->get_position (camera);
                            dir = camera->iface->screen2world (camera, width/2, height/2);
                            const struct vox_node* leaf =
                                vox_ray_tree_intersection (tree, origin, dir, &inter);
                            if (leaf != NULL)
                            {
                                if (event.key.keysym.scancode == config.insert)
                                    amend_box (&tree, inter, 5, 1);
                                else
                                    amend_box (&tree, inter, 5, 0);
                                vox_context_set_scene (ctx, tree);
                            }
                        });
                } else if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    goto end;
                } else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                    /*
                      Rebuild the tree asynchronously and then
                      update the context.
                    */
                    dispatch_group_async (tree_group,
                                          dispatch_get_global_queue
                                          (DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
                                              struct vox_node *new_tree = vox_rebuild_tree (tree);
                                              if (new_tree != NULL) {
                                                  dispatch_sync (tree_queue, ^{
                                                          vox_destroy_tree (tree);
                                                          tree = new_tree;
                                                          vox_context_set_scene (ctx, tree);
                                                          printf ("Tree rebuilt\n");
                                                      });
                                              }
                                          });
                } else if (event.key.keysym.scancode == SDL_SCANCODE_E) {
                    /*
                      Freeze the tree asynchronously and then
                      update the context.
                    */
                    dispatch_group_async (tree_group,
                                          dispatch_get_global_queue
                                          (DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
                                              struct vox_node *new_tree = vox_freeze_tree (tree);
                                              if (new_tree != NULL) {
                                                  dispatch_sync (tree_queue, ^{
                                                          vox_destroy_tree (tree);
                                                          tree = new_tree;
                                                          vox_context_set_scene (ctx, tree);
                                                          printf ("Tree frozen\n");
                                                      });
                                              }
                                          });
                } else if (event.key.keysym.scancode == SDL_SCANCODE_F11) {
                    vox_screenshot (ctx, NULL);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                    unsigned int mode = quality & VOX_QUALITY_MODE_MASK;
                    const char *msg;
                    if (mode == VOX_QUALITY_BEST) {
                        msg = "fast";
                        mode = VOX_QUALITY_FAST;
                    } else {
                        msg = "best";
                        mode = VOX_QUALITY_BEST;
                    }

                    printf ("Changing to %s mode\n", msg);
                    vox_context_set_quality (ctx, mode);
                    quality = mode;
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

        if (keystate[config.walk_forwards]) {
            step[1] += 5;
        } else if (keystate[config.walk_backwards]) {
            step[1] -= 5;
        }
        if (keystate[config.walk_right]) {
            step[0] += 5;
        } else if (keystate[config.walk_left]) {
            step[0] -= 5;
        }
        if (keystate[config.fly_up]) {
            step[2] += 5;
        } else if (keystate[config.fly_down]) {
            step[2] -= 5;
        }
        if (keystate[config.tilt_left]) {
            rot_delta[1] += 0.01;
        } else if (keystate[config.tilt_right]) {
            rot_delta[1] -= 0.01;
        }
        camera->iface->move_camera (camera, step);

        vox_cd_collide (cd);
        struct vox_fps_info fps_info = fps_controller();
        if (vox_fpsstatus_updated (fps_info.status)) {
            printf ("Frames per second: %i\n", vox_fpsstatus_fps (fps_info.status));
        }

        SDL_GetRelativeMouseState (&x, &y);
        rot_delta[2] = -xspeed*x;
        rot_delta[0] = +yspeed*y;
        camera->iface->rotate_camera (camera, rot_delta);
    }

end:
    if (ctx != NULL) {
        vox_destroy_context (ctx);
    }
    if (camera != NULL) {
        camera->iface->destroy_camera (camera);
    }
    if (tree != NULL) {
        vox_destroy_tree (tree);
    }
    if (SDL_WasInit(0)) {
        SDL_Quit();
    }
#if USE_GCD
    if (tree_queue != NULL) {
        dispatch_release (tree_queue);
    }
    if (tree_group != NULL) {
        dispatch_release (tree_group);
    }
#endif
    if (fps_controller != NULL) {
        vox_destroy_fps_controller (fps_controller);
    }
    if (cd != NULL) {
        vox_destroy_cd (cd);
    }

    return 0;
}
