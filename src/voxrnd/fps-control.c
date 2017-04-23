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
    __block struct vox_fps_info fps_info;
    memset (&fps_info, 0, sizeof (struct vox_fps_info));
    fps_info.total_time = SDL_GetTicks();
    vox_fps_controller_t controller = ^{
        /*
         * Call this block in a loop in the main thread.
         * It will put a rendering loop to sleep when needed.
         */
        long delta, tmp;
        Uint32 new_time;

        fps_info.trigger = 0;
        fps_info.counter++;
        SDL_Delay ((Uint32)fps_info.delay);
        new_time = SDL_GetTicks();
        fps_info.frame_time = new_time - fps_info.current_time;

        if (fps)
        {
            delta = fps_info.frame_time - fps_info.delay;
            tmp = (1<<16) - (fps<<16)*(long)fps_info.frame_time/1000;
            delta *= tmp;
            tmp = delta >> 16;
            if (tmp == 0) delta >>= 15;
            else delta = tmp;
            fps_info.delay += (int)delta;
            if (fps_info.delay < 0) fps_info.delay = 0;
        }

        if (new_time - fps_info.total_time > 1000)
        {
            fps_info.fps = fps_info.counter;
            fps_info.counter = 0;
            fps_info.total_time = new_time;
            fps_info.trigger = 1;
        }
        fps_info.current_time = SDL_GetTicks();

        return &fps_info;
    };

    return Block_copy (controller);
}

void vox_destroy_fps_controller (vox_fps_controller_t controller)
{
    Block_release (controller);
}
