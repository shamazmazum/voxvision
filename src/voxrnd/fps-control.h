/**
   @file fps-control.h
   @brief FPS controller
**/
#ifndef FRAME_COUNTER_H
#define FRAME_COUNTER_H

/**
   \brief FPS controller type
**/
typedef int(^vox_fps_controller_t)(void);

/**
   \brief Make FPS controller.

   FPS controller is a block that must be called once in a rendering
   loop. It will put a thread to sleep to achieve a specified amount of
   frames per second. Once in a second it will return an actual value of
   FPS. In all other cases it will return 0.

   \param fps A desired FPS value. If it's zero, only count FPS, do not put
   a thread to sleep.
   \return Actual FPS or zero.
**/
vox_fps_controller_t vox_make_fps_controller (int fps);

/**
   \brief Destroy FPS controller created earlier.
**/
void vox_destroy_fps_controller (vox_fps_controller_t controller);

#endif
