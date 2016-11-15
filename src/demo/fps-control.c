#include <SDL.h>
#include <Block.h>
#include "fps-control.h"

/*
 * This FPS controller is not very smart. Will work on it later.
 * It takes into account the rendering time of the previous frame
 * to calculate a delay delta for the current frame. The only good
 * thing in it is that it has negative feedback, i.e. if the frame was
 * rendered faster than desired, delay delta is positive and the next
 * frame will be rendered slower, and, on the other side, if it was
 * rendered slower than desired, the next will be rendered faster.
 */
fps_controller_t make_fps_controller (int fps)
{
    __block Uint32 total_time = SDL_GetTicks();
    __block Uint32 frame_time = total_time;
    __block int counter = 0;
    __block int delay = 0;
    fps_controller_t controller = ^{
        /*
         * Call this block in a loop in the main thread.
         * It will put a rendering loop to sleep when needed
         * and also return fps count each second.
         */
        int actual_fps = 0;
        long delta, mul;
        Uint32 new_time;

        counter++;
        SDL_Delay ((Uint32)delay);
        new_time = SDL_GetTicks();

        if (fps)
        {
            frame_time = new_time - frame_time;
            delta = frame_time - delay;
            mul = (1<<16) - (fps<<16)*(long)frame_time/1000;
            delta *= mul;
            delta >>= 16;
            delay += (int)delta;
            if (delay < 0) delay = 0;
        }

        if (new_time - total_time > 1000)
        {
            actual_fps = counter;
            counter = 0;
            total_time = new_time;
        }
        frame_time = SDL_GetTicks();

        return actual_fps;
    };

    return Block_copy (controller);
}

void destroy_fps_controller (fps_controller_t controller)
{
    Block_release (controller);
}
