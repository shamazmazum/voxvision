#include <Block.h>
#include <strings.h>
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
vox_fps_controller_t vox_make_fps_controller (int fps)
{
    __block unsigned int counter = 0;
    __block int delay = 0;
    __block unsigned int actual_fps = 0;
    __block Uint32 total_time = SDL_GetTicks();
    __block Uint32 current_time = total_time;

    vox_fps_controller_t controller = ^{
        /*
         * Call this block in a loop in the main thread.
         * It will put a rendering loop to sleep when needed.
         */
        long delta, tmp;
        Uint32 new_time;
        Uint32 frame_time;
        struct vox_fps_info fps_info;
        unsigned int updated = 0;

        counter++;
        SDL_Delay ((Uint32)delay);
        new_time = SDL_GetTicks();
        frame_time = new_time - current_time;

        if (fps)
        {
            delta = frame_time - delay;
            tmp = (1<<16) - (fps<<16)*(long)frame_time/1000;
            delta *= tmp;
            tmp = delta >> 16;
            if (tmp == 0) delta >>= 15;
            else delta = tmp;
            delay += (int)delta;
            if (delay < 0) delay = 0;
        }

        if (new_time - total_time > 1000)
        {
            actual_fps = counter;
            counter = 0;
            total_time = new_time;
            updated = 1;
        }
        current_time = SDL_GetTicks();

        fps_info.status = vox_fpsstatus (actual_fps, updated);
        fps_info.frame_time = frame_time;
        return fps_info;
    };

    return Block_copy (controller);
}

void vox_destroy_fps_controller (vox_fps_controller_t controller)
{
    Block_release (controller);
}
