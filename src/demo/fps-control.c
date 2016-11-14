#include <SDL.h>
#include <Block.h>
#include "fps-control.h"

fps_controller_t make_fps_controller (int fps)
{
    __block Uint32 time = SDL_GetTicks();
    __block int counter = 0;
    __block int delay = 0;
    fps_controller_t controller = ^{
        /*
         * Call this block in a loop in the main thread.
         * It will put a rendering loop to sleep when needed
         * and also return fps count each second.
         */
        counter++;
        Uint32 diff = SDL_GetTicks() - time;
        int actual_fps = 0;
        int delta;

        if (diff > 1000)
        {
            if (fps)
            {
                delta = counter - fps;
                delta *= (1000 - delay*counter);
                delta /= counter*counter;
                delay += delta;
                if (delay < 0) delay = 0;
            }

            actual_fps = counter;
            counter = 0;
            time = SDL_GetTicks();
        }
        SDL_Delay ((Uint32)delay);

        return actual_fps;
    };

    return Block_copy (controller);
}

void destroy_fps_controller (fps_controller_t controller)
{
    Block_release (controller);
}
