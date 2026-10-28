#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <iniparser.h>
#include <SDL2/SDL.h>
#include <voxvision.h>

struct settings
{
    int          window_width;
    int          window_height;
    int          fps;
    unsigned int quality;
    float        xspeed;
    float        yspeed;
    vox_dot      voxel_size;
    vox_dot      camera_pos;
    float        camera_fov;
    float        threshold;

    // Controls
    SDL_Scancode tilt_left;
    SDL_Scancode tilt_right;

    SDL_Scancode walk_left;
    SDL_Scancode walk_right;
    SDL_Scancode walk_forwards;
    SDL_Scancode walk_backwards;
    SDL_Scancode fly_up;
    SDL_Scancode fly_down;

    SDL_Scancode insert;
    SDL_Scancode delete;
};

int load_configuration (const char *filename, struct settings *config);
void init_configuration (struct settings *config);

#endif
