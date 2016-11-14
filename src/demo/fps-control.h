#ifndef FRAME_COUNTER_H
#define FRAME_COUNTER_H

typedef int(^fps_controller_t)(void);

fps_controller_t make_fps_controller (int fps);
void destroy_fps_controller (fps_controller_t controller);

#endif
