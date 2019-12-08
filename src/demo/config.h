#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <iniparser.h>
#include <SDL2/SDL.h>

struct controls
{
    // Head movement
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
extern struct controls global_controls;

struct settings
{
    int window_width;
    int window_height;
    int fps;
    unsigned int quality;
    float xspeed;
    float yspeed;
};
extern struct settings global_settings;

int load_configuration (const char *filename);

// Underscore is to distinguish iniparser's functions from ours
int _iniparser_getvector3_int (dictionary *dict, const char *entry, int result[]);
int _iniparser_getvector3_float (dictionary *dict, const char *entry, float result[]);

#endif
