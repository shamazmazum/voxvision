#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <voxengine.h>
#include <SDL2/SDL.h>

static void usage()
{
    fprintf (stderr, "Usase: voxvision-engine [-w width] [-h height] [-f fps] "
                     "[-q quality] [-m] -s script\n");
    fprintf (stderr, "quality = fast | best | adaptive\n");
    exit (EXIT_FAILURE);
}

static int choose_quality (const char *quality_str)
{
    int quality;

    if (strcmp (quality_str, "fast") == 0) quality = VOX_QUALITY_FAST;
    else if (strcmp (quality_str, "best") == 0) quality = VOX_QUALITY_BEST;
    else if (strcmp (quality_str, "adaptive") == 0) quality = VOX_QUALITY_ADAPTIVE;
    else quality = -1;

    return quality;
}

int main (int argc, char *argv[])
{
    int ch, width = 800, height = 600, fps = 30;
    const char *script = NULL;
    char *endptr;
    int quality = VOX_QUALITY_ADAPTIVE;
    int merge_rays = 0;

    while ((ch = getopt (argc, argv, "w:h:s:f:q:m")) != -1)
    {
        switch (ch)
        {
        case 'w':
            width = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') usage();
            break;
        case 'h':
            height = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') usage();
            break;
        case 's':
            script = optarg;
            break;
        case 'f':
            fps = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') usage();
            break;
        case 'q':
            quality = choose_quality (optarg);
            break;
        case 'm':
            merge_rays = 1;
            break;
        default:
            usage();
        }
    }

    argc -= optind;
    argv += optind;

    if (script == NULL) usage ();

    struct vox_engine *engine = vox_create_engine (width, height, script, argc,
                                                   (argc)? argv: NULL);
    if (engine == NULL) {
        fprintf (stderr, "Cannot create engine\n");
        return 1;
    }

    quality |= (merge_rays)? VOX_QUALITY_RAY_MERGE: 0;
    if (!vox_context_set_quality (engine->ctx, quality))
        fprintf (stderr, "Error setting quality. Falling back to adaptive mode\n");

    vox_fps_controller_t fps_controller = vox_make_fps_controller (fps);
    SDL_EventState (SDL_MOUSEMOTION, SDL_DISABLE);
    SDL_SetRelativeMouseMode (SDL_TRUE);
    vox_engine_status status;

    while (1)
    {
        status = vox_engine_tick (engine);
        struct vox_fps_info fps_info = fps_controller();
        if (vox_fpsstatus_updated (fps_info.status))
            printf ("Frames per second: %i\n", vox_fpsstatus_fps (fps_info.status));
        if (vox_engine_quit_requested (status)) goto end;
    }
end:
    vox_destroy_engine (engine);
    
    return 0;
}
