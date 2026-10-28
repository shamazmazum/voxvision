#include <iniparser.h>
#include <string.h>
#include <stdio.h>
#include <voxrnd.h>
#include "config.h"

// Default settings
const static struct settings default_settings = {
    .window_width   = 800,
    .window_height  = 600,
    .fps            = 30,
    .quality        = VOX_QUALITY_FAST,
    .xspeed         = 0.01,
    .yspeed         = 0.01,
    .voxel_size     = (vox_dot) {1, 1, 1, 0},
    .camera_pos     = (vox_dot) {0, 0, 0, 0},
    .camera_fov     = 1.0,
    .threshold      = 0.5,

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

static void set_control (dictionary *dict, const char *control, SDL_Scancode *place) {
    const char *name = iniparser_getstring (dict, control, NULL);
    if (name != NULL) {
        *place = SDL_GetScancodeFromName (name);
    }
}

static int _iniparser_getvector3_float (dictionary *dict,
                                        const char *entry,
                                        vox_dot *result) {
    const char *value;
    vox_dot a;
    float x,y,z;
    int n;

    value = iniparser_getstring (dict, entry, NULL);
    if (value == NULL) {
        return 0;
    }

    n = sscanf (value, "<%f, %f, %f>", &x, &y, &z);
    if (n != 3) {
        return 0;
    }

    a = (vox_dot) {x, y, z, 0};
    *result = a;
    return 1;
}


void init_configuration (struct settings *config) {
    memcpy (config, &default_settings, sizeof (struct settings));
}

int load_configuration (const char *filename, struct settings *config) {
    dictionary *dict = iniparser_load (filename);
    if (dict == NULL) {
        return 0;
    }

    set_control (dict, "Controls:TiltLeft",      &config->tilt_left);
    set_control (dict, "Controls:TiltRight",     &config->tilt_right);

    set_control (dict, "Controls:WalkLeft",      &config->walk_left);
    set_control (dict, "Controls:WalkRight",     &config->walk_right);
    set_control (dict, "Controls:WalkForwards",  &config->walk_forwards);
    set_control (dict, "Controls:WalkBackwards", &config->walk_backwards);
    set_control (dict, "Controls:FlyUp",         &config->fly_up);
    set_control (dict, "Controls:FlyDown",       &config->fly_down);

    set_control (dict, "Controls:Insert",        &config->insert);
    set_control (dict, "Controls:Delete",        &config->delete);

    config->xspeed = iniparser_getdouble (dict, "Controls:MouseXSpeed", config->xspeed);
    config->yspeed = iniparser_getdouble (dict, "Controls:MouseYSpeed", config->yspeed);
    config->window_width = iniparser_getint (dict, "Window:Width", config->window_width);
    config->window_height = iniparser_getint (dict, "Window:Height", config->window_height);
    config->fps = iniparser_getint (dict, "Renderer:FPS", config->fps);
    const char *quality = iniparser_getstring (dict, "Renderer:Quality", "Fast");
    if (strcmp (quality, "Best") == 0) {
        config->quality = VOX_QUALITY_BEST;
    } else if (strcmp (quality, "Fast") == 0) {
        config->quality = VOX_QUALITY_FAST;
    } else {
        fprintf (stderr, "Wrong quality: %s\n", quality);
    }
    _iniparser_getvector3_float (dict, "Renderer:VoxelSize", &config->voxel_size);
    _iniparser_getvector3_float (dict, "Camera:Position", &config->camera_pos);
    config->camera_fov = iniparser_getdouble (dict, "Camera:FoV", config->camera_fov);
    config->threshold = iniparser_getdouble (dict, "Dataset:Threshold", config->threshold);

    iniparser_freedict (dict);
    return 1;
}
