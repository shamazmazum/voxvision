#include <voxengine.h>
#include <SDL2/SDL.h>

int main (int argc, char *argv[])
{
    struct vox_engine *engine = vox_create_engine (&argc, &argv);
    if (engine == NULL) return 1;

    while (1)
    {
        SDL_Event event;

        vox_engine_tick (engine);
        if (vox_fpsstatus_updated (engine->fps_info.status))
            printf ("Frames per second: %i\n", vox_fpsstatus_fps (engine->fps_info.status));
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
