/**
   @file fps-control.h
   @brief FPS controller
**/
#ifndef FRAME_COUNTER_H
#define FRAME_COUNTER_H

#include <SDL2/SDL.h>
#ifndef VOXRND_SOURCE
/**
   \brief Frame counter info.

    This structure is returned by FPS counter.
**/
struct vox_fps_info
{
    int trigger;
    /**<
       \brief Becomes 1 once in a second, 0 all other time.

       Indicates update in the structure.
    */
    int fps; /**< \brief Actual FPS value */
    Uint32 frame_time;
    /**<
       \brief Time taken to render the previous frame.

       Given in SDL ticks.
    */
};
#else /* VOXRND_SOURCE */
struct vox_fps_info
{
    int trigger;
    int fps;
    Uint32 frame_time;
    Uint32 current_time;
    Uint32 total_time;
    int counter;
    int delay;
};
#endif /* VOXRND_SOURCE */

/**
   \brief FPS controller type
**/
typedef struct vox_fps_info* (^vox_fps_controller_t)(void);

/**
   \brief Make FPS controller.

   FPS controller is a block that must be called once in a rendering
   loop. It will put a thread to sleep to achieve a specified amount of
   frames per second. It returns a pointer to its internal structure
   vox_fps_info (see documentation). This structure lives as much as FPS
   controller lives and must not be freed by user himeself.

   \param fps A desired FPS value. If it's zero, only count FPS, do not put
   a thread to sleep.
   \return Pointer to info structure.
**/
vox_fps_controller_t vox_make_fps_controller (int fps);

/**
   \brief Destroy FPS controller created earlier.
**/
void vox_destroy_fps_controller (vox_fps_controller_t controller);

#endif
