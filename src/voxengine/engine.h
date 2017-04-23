/**
   @file engine.h
   @brief Lua engine
**/
#ifndef ENGINE_H
#define ENGINE_H

#include "../voxtrees/tree.h"
#include "../voxrnd/camera.h"
#include "../voxrnd/fps-control.h"
#include "../voxrnd/renderer.h"
#include <SDL2/SDL.h>

#ifdef VOXENGINE_SOURCE
struct vox_engine {
    struct vox_camera *camera;
    struct vox_node *tree;
    struct vox_rnd_ctx *ctx;
    struct vox_fps_info fps_info;

    lua_State *L;
    int width, height;
    const char *script;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *surface;

    vox_fps_controller_t fps_controller;
};
#else /* VOXENGINE_SOURCE */
struct vox_engine {
    struct vox_camera *camera;
    struct vox_node *tree;
    struct vox_rnd_ctx *ctx;
    struct vox_fps_info fps_info;
};
#endif /* VOXENGINE_SOURCE */

struct vox_engine* vox_create_engine (int *argc, char **argv[]);
void vox_engine_tick (struct vox_engine *engine);
void vox_destroy_engine (struct vox_engine *engine);

#endif
