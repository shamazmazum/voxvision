#include <unistd.h>
#include <voxengine.h>
#include <SDL2/SDL.h>

static void usage()
{
    fprintf (stderr, "Usase: voxvision-engine [-w width] [-h height] [-f fps] -s script\n");
}

int main (int argc, char *argv[])
{
    int ch, width = 800, height = 600, fps = 30;
    const char *script = NULL;
    while ((ch = getopt (argc, argv, "w:h:s:f:")) != -1)
    {
        switch (ch)
        {
        case 'w':
            width = strtol (optarg, NULL, 10);
            break;
        case 'h':
            height = strtol (optarg, NULL, 10);
            break;
        case 's':
            script = optarg;
            break;
        case 'f':
            fps = strtol (optarg, NULL, 10);
            break;
        case '?':
        default:
            usage();
            return 1;
        }
    }

    argc -= optind;
    argv += optind;

    if (script == NULL)
    {
        usage ();
        return 1;
    }

    struct vox_engine *engine = vox_create_engine (width, height);
    if (engine == NULL) return 1;
    vox_fps_controller_t fps_controller = vox_make_fps_controller (fps);
    vox_engine_load_script (engine, script);

    while (1)
    {
        SDL_Event event;

        vox_engine_tick (engine);
        struct vox_fps_info fps_info = fps_controller();
        if (vox_fpsstatus_updated (fps_info.status))
            printf ("Frames per second: %i\n", vox_fpsstatus_fps (fps_info.status));
        if (SDL_PollEvent (&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_Q) goto end;
                break;
            case SDL_QUIT:
                goto end;
            }
        }
    }
end:
    vox_destroy_engine (engine);
    
    return 0;
}
