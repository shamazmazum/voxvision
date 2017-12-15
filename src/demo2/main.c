#include <unistd.h>
#include <stdlib.h>
#include <voxengine.h>
#include <SDL2/SDL.h>

static void usage()
{
    fprintf (stderr, "Usase: voxvision-engine [-w width] [-h height] [-f fps] -s script\n");
    exit (EXIT_FAILURE);
}

int main (int argc, char *argv[])
{
    int ch, width = 800, height = 600, fps = 30;
    const char *script = NULL;
    char *endptr;
    while ((ch = getopt (argc, argv, "w:h:s:f:")) != -1)
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
        case '?':
        default:
            usage();
        }
    }

    argc -= optind;
    argv += optind;

    if (script == NULL) usage ();

    struct vox_engine *engine = vox_create_engine (width, height, script);
    if (engine == NULL) return 1;
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
