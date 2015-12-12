#ifndef _CONFIG_H_
#define _CONFIG_H_

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
};
extern struct controls global_controls;

struct settings
{
    int window_width;
    int window_height;
};
extern struct settings global_settings;

int load_configuration (const char *filename);

#endif
