#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <iniparser.h>

struct controls
{
    // Head movement
    int look_up;
    int look_down;
    int look_left;
    int look_right;
    int tilt_left;
    int tilt_right;

    int walk_left;
    int walk_right;
    int walk_forwards;
    int walk_backwards;
    int fly_up;
    int fly_down;

    int shrink;
    int grow;

    int insert;
    int delete;

    int toggle_camera;
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
