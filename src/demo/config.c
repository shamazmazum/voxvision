#include <SDL2/SDL.h>
#include <iniparser.h>
#include <string.h>
#include "config.h"

// Initialize global controls with defaults.
struct controls global_controls =
{
    .tilt_left      = SDL_SCANCODE_Z,
    .tilt_right     = SDL_SCANCODE_X,

    .walk_left      = SDL_SCANCODE_A,
    .walk_right     = SDL_SCANCODE_D,
    .walk_forwards  = SDL_SCANCODE_W,
    .walk_backwards = SDL_SCANCODE_S,
    .fly_up         = SDL_SCANCODE_1,
    .fly_down       = SDL_SCANCODE_2,

    .insert         = SDL_SCANCODE_I,
    .delete         = SDL_SCANCODE_O,
};

// And the same for the settings.
struct settings global_settings =
{
    .window_width  = 800,
    .window_height = 600,
    .fps = 30,
    .xspeed         = 0.01,
    .yspeed         = 0.01,
    .light_radius   = 100
};

static void set_control (dictionary *dict, const char *control, int *place)
{
    const char *name = iniparser_getstring (dict, control, NULL);
    if (name != NULL) *place = SDL_GetScancodeFromName (name);
}

int load_configuration (const char *filename)
{
    dictionary *dict = iniparser_load (filename);
    if (dict == NULL) return -1;

    set_control (dict, "Controls:TiltLeft", &global_controls.tilt_left);
    set_control (dict, "Controls:TiltRight", &global_controls.tilt_right);

    set_control (dict, "Controls:WalkLeft", &global_controls.walk_left);
    set_control (dict, "Controls:WalkRight", &global_controls.walk_right);
    set_control (dict, "Controls:WalkForwards", &global_controls.walk_forwards);
    set_control (dict, "Controls:WalkBackwards", &global_controls.walk_backwards);
    set_control (dict, "Controls:FlyUp", &global_controls.fly_up);
    set_control (dict, "Controls:FlyDown", &global_controls.fly_down);

    set_control (dict, "Controls:Insert", &global_controls.insert);
    set_control (dict, "Controls:Delete", &global_controls.delete);

    global_settings.xspeed = iniparser_getdouble (dict, "Controls:MouseXSpeed", global_settings.yspeed);
    global_settings.yspeed = iniparser_getdouble (dict, "Controls:MouseYSpeed", global_settings.yspeed);
    global_settings.window_width = iniparser_getint (dict, "Window:Width", global_settings.window_width);
    global_settings.window_height = iniparser_getint (dict, "Window:Height", global_settings.window_height);
    global_settings.fps = iniparser_getint (dict, "Renderer:FPS", global_settings.fps);
    global_settings.light_radius = iniparser_getdouble (dict, "Renderer:LightRadius",
                                                        global_settings.light_radius);

    iniparser_freedict (dict);
    return 0;
}

int _iniparser_getvector3_int (dictionary *dict, const char *entry, int result[])
{
    const char *value;
    int a[3];
    int n;

    value = iniparser_getstring (dict, entry, NULL);
    if (value == NULL) return 0;

    n = sscanf (value, "<%i, %i, %i>", &(a[0]), &(a[1]), &(a[2]));
    if (n != 3) return 0;

    memcpy (result, a, sizeof(int)*3);
    return 1;
}

int _iniparser_getvector3_float (dictionary *dict, const char *entry, float result[])
{
    const char *value;
    float a[3];
    int n;

    value = iniparser_getstring (dict, entry, NULL);
    if (value == NULL) return 0;

    n = sscanf (value, "<%f, %f, %f>", &(a[0]), &(a[1]), &(a[2]));
    if (n != 3) return 0;

    memcpy (result, a, sizeof(float)*3);
    return 1;
}
