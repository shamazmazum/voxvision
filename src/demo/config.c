#include <SDL/SDL.h>
#include <iniparser.h>
#include <string.h>
#include "config.h"

// Initialize global controls with defaults.
struct controls global_controls =
{
    .look_up        = SDLK_UP,
    .look_down      = SDLK_DOWN,
    .look_left      = SDLK_LEFT,
    .look_right     = SDLK_RIGHT,
    .tilt_left      = SDLK_z,
    .tilt_right     = SDLK_x,

    .walk_left      = SDLK_a,
    .walk_right     = SDLK_d,
    .walk_forwards  = SDLK_w,
    .walk_backwards = SDLK_s,
    .fly_up         = SDLK_1,
    .fly_down       = SDLK_2,

    .shrink         = SDLK_h,
    .grow           = SDLK_g,

    .insert         = SDLK_i,
    .delete         = SDLK_o
};

// And the same for the settings.
struct settings global_settings =
{
    .window_width  = 800,
    .window_height = 600
};

static void set_control (dictionary *dict, const char *control, unsigned short *place)
{
    const char *value = iniparser_getstring (dict, control, NULL);
    if (value != NULL)
    {
        if (strcmp (value, "LeftArrow") == 0) *place = SDLK_LEFT;
        else if (strcmp (value, "RightArrow") == 0) *place = SDLK_RIGHT;
        else if (strcmp (value, "UpArrow") == 0) *place = SDLK_UP;
        else if (strcmp (value, "DownArrow") == 0) *place = SDLK_DOWN;
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
