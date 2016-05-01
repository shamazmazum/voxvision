#include <SDL2/SDL.h>
#include <iniparser.h>
#include <string.h>
#include "config.h"

// Initialize global controls with defaults.
struct controls global_controls =
{
    .look_up        = SDL_SCANCODE_UP,
    .look_down      = SDL_SCANCODE_DOWN,
    .look_left      = SDL_SCANCODE_LEFT,
    .look_right     = SDL_SCANCODE_RIGHT,
    .tilt_left      = SDL_SCANCODE_Z,
    .tilt_right     = SDL_SCANCODE_X,

    .walk_left      = SDL_SCANCODE_A,
    .walk_right     = SDL_SCANCODE_D,
    .walk_forwards  = SDL_SCANCODE_W,
    .walk_backwards = SDL_SCANCODE_S,
    .fly_up         = SDL_SCANCODE_1,
    .fly_down       = SDL_SCANCODE_2,

    .shrink         = SDL_SCANCODE_H,
    .grow           = SDL_SCANCODE_G,

    .insert         = SDL_SCANCODE_I,
    .delete         = SDL_SCANCODE_O
};

// And the same for the settings.
struct settings global_settings =
{
    .window_width  = 800,
    .window_height = 600
};

static void set_control (dictionary *dict, const char *control, unsigned short *place)
{
    const char *value = iniparser_getstring (dict, control, "default");
    if (strcmp (value, "default"))
    {
        if (strcmp (value, "LeftArrow") == 0) *place = SDL_SCANCODE_LEFT;
        else if (strcmp (value, "RightArrow") == 0) *place = SDL_SCANCODE_RIGHT;
        else if (strcmp (value, "UpArrow") == 0) *place = SDL_SCANCODE_UP;
        else if (strcmp (value, "DownArrow") == 0) *place = SDL_SCANCODE_DOWN;
        else if (strlen (value) == 1) *place = value[0];
    }
}

int load_configuration (const char *filename)
{
    dictionary *dict = iniparser_load (filename);
    if (dict == NULL) return -1;

    set_control (dict, "Controls:LookUp", &global_controls.look_up);
    set_control (dict, "Controls:LookDown", &global_controls.look_down);
    set_control (dict, "Controls:LookLeft", &global_controls.look_left);
    set_control (dict, "Controls:LookRight", &global_controls.look_right);
    set_control (dict, "Controls:TiltLeft", &global_controls.tilt_left);
    set_control (dict, "Controls:TiltRight", &global_controls.tilt_right);

    set_control (dict, "Controls:WalkLeft", &global_controls.walk_left);
    set_control (dict, "Controls:WalkRight", &global_controls.walk_right);
    set_control (dict, "Controls:WalkForwards", &global_controls.walk_forwards);
    set_control (dict, "Controls:WalkBackwards", &global_controls.walk_backwards);
    set_control (dict, "Controls:FlyUp", &global_controls.fly_up);
    set_control (dict, "Controls:FlyDown", &global_controls.fly_down);

    global_settings.window_width = iniparser_getint (dict, "Window:Width", global_settings.window_width);
    global_settings.window_height = iniparser_getint (dict, "Window:Height", global_settings.window_height);
    iniparser_freedict (dict);
    return 0;
}
