#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <iniparser.h>

struct controls
{
    // Head movement
    unsigned short look_up;
    unsigned short look_down;
    unsigned short look_left;
    unsigned short look_right;
    unsigned short tilt_left;
    unsigned short tilt_right;

    unsigned short walk_left;
    unsigned short walk_right;
    unsigned short walk_forwards;
    unsigned short walk_backwards;
    unsigned short fly_up;
    unsigned short fly_down;

    unsigned short shrink;
    unsigned short grow;

    unsigned short insert;
    unsigned short delete;

    unsigned short toggle_camera;
};
extern struct controls global_controls;

struct settings
{
    int window_width;
    int window_height;
};
extern struct settings global_settings;

int load_configuration (const char *filename);

// Underscore is to distinguish iniparser's functions from ours
int _iniparser_getvector3_int (dictionary *dict, const char *entry, int result[]);
int _iniparser_getvector3_float (dictionary *dict, const char *entry, float result[]);

#endif
