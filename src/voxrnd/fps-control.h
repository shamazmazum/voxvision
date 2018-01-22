/**
   @file fps-control.h
   @brief FPS controller
**/
#ifndef FRAME_COUNTER_H
#define FRAME_COUNTER_H
#include <SDL2/SDL.h>

/**
   \brief true if FPS controller status was updated.
**/
#define vox_fpsstatus_updated(status) (status)&1

/**
   \brief Get actual FPS value.
**/
#define vox_fpsstatus_fps(status) (status)>>1

/**
   \brief Construct FPS status.
**/
#define vox_fpsstatus(fps, updated) ((fps)<<1 | ((updated) ? 1: 0))

/**
   \brief Frame counter info.

    This structure is returned by FPS counter.
**/
struct vox_fps_info
{
    unsigned int status;
    /**<
       \brief FPS controller status.

       See vox_fpsstatus_updated() and vox_fpsstatus_fps().
    */
    Uint32 frame_time;
    /**<
       \brief Time taken to render the previous frame.

       Given in SDL ticks (milliseconds).
    */
};

/**
   \brief FPS controller type
**/
typedef struct vox_fps_info (^vox_fps_controller_t)(void);

/**
   \brief Make FPS controller.

   FPS controller is a block that must be called once in a rendering
   loop. It will put a thread to sleep to achieve a specified amount of
   frames per second. This block returns FPS controller status (see vox_fps_info).

   \param fps A desired FPS value. If it's zero, only count FPS, do not put
   a thread to sleep.
   \return FPS controller status.
**/
vox_fps_controller_t vox_make_fps_controller (unsigned int fps);

/**
   \brief Destroy FPS controller created earlier.
**/
void vox_destroy_fps_controller (vox_fps_controller_t controller);

#endif
