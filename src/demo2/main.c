#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <voxengine.h>
#include <SDL2/SDL.h>

static void usage() {
    fprintf (stderr, "Usase: voxvision-engine [-w width] [-h height] "
                     "[-q quality] [-m ray-merge-mode] [-d] -s script\n");
    fprintf (stderr, "quality = fast | best\n");
    exit (EXIT_FAILURE);
}

static int choose_quality (const char *quality_str) {
    int quality;

    if (strcmp (quality_str, "fast") == 0) {
        quality = VOX_QUALITY_FAST;
    } else if (strcmp (quality_str, "best") == 0) {
        quality = VOX_QUALITY_BEST;
    } else {
        quality = -1;
    }

    return quality;
}

static void ensure_datadir_exists() {
    int res;
    if (getenv ("VOXVISION_DATA") != NULL) {
        res = mkdir (getenv ("VOXVISION_DATA"), 0750);
        if (res == -1 && errno != EEXIST) {
            fprintf (stderr, "Cannot create voxvision home directory, exiting\n");
            exit (EXIT_FAILURE);
        }
    }
}

int main (int argc, char *argv[]) {
    int ch;
    const char *script = NULL;
    char *endptr;
    int quality = VOX_QUALITY_FAST;
    int merge_rays = 0;
    unsigned int flags = 0;
    int width = 800, height = 600;

    while ((ch = getopt (argc, argv, "w:h:s:q:d")) != -1) {
        switch (ch) {
        case 'w':
            width = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage();
            }
            break;
        case 'h':
            height = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage();
            }
            break;
        case 's':
            script = optarg;
            break;
        case 'q':
            quality = choose_quality (optarg);
            break;
        case 'd':
            flags |= VOX_ENGINE_DEBUG;
            break;
        default:
            usage();
        }
    }

    argc -= optind;
    argv += optind;

    if (script == NULL) {
        usage ();
    }

    if (quality == -1 || merge_rays == -1) {
        usage();
    }

    if (width < 0 || height < 0) {
        usage ();
    }

    ensure_datadir_exists();

    if (!(flags & VOX_ENGINE_DEBUG)) {
        if (SDL_Init (SDL_INIT_VIDEO) != 0) {
            fprintf (stderr, "Cannot init SDL: %s\n", SDL_GetError());
            return 1;
        }

        SDL_EventState (SDL_MOUSEMOTION, SDL_DISABLE);
        SDL_SetRelativeMouseMode (SDL_TRUE);
    }

    struct vox_engine *engine = vox_create_engine (width, height, flags, script,
                                                   argc, (argc)? argv: NULL);
    if (engine == NULL) {
        fprintf (stderr, "Cannot create engine\n");
        return 1;
    }

    /* The context is not created in debugging mode. */
    if (engine->ctx != NULL) {
        quality |= merge_rays;
        if (!vox_context_set_quality (engine->ctx, quality)) {
            fprintf (stderr, "Error setting quality. Falling back to adaptive mode\n");
        }
    }

    vox_engine_status status;
    while (1) {
        status = vox_engine_tick (engine);
        if (vox_engine_quit_requested (status)) {
            goto end;
        }
    }

end:
    vox_destroy_engine (engine);
    
    return 0;
}
