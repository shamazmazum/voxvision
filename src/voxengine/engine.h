/**
   @file engine.h
   @brief Lua engine
**/
#ifndef ENGINE_H
#define ENGINE_H
#include <SDL2/SDL.h>

#include "../voxtrees/tree.h"
#include "../voxrnd/camera.h"
#include "../voxrnd/fps-control.h"
#include "../voxrnd/renderer.h"
#include "../voxrnd/cd.h"

#ifdef VOXENGINE_SOURCE
struct vox_engine {
    struct vox_camera *camera;
    struct vox_node *tree;
    struct vox_rnd_ctx *ctx;
    struct vox_fps_info fps_info;

    lua_State *L;
    int width, height;
    const char *script;

    vox_fps_controller_t fps_controller;
    struct vox_cd *cd;
};
#else /* VOXENGINE_SOURCE */

/**
   \brief Voxengine instance.

   This is a part of voxengine structure visible to user. You must create
   voxengine with vox_create_engine.
**/

struct vox_engine {
    struct vox_camera *camera;
    /**< \brief Camera used by an engine. */
    struct vox_node *tree;
    /**< \brief Tree used by an engine. **/
    struct vox_rnd_ctx *ctx;
    /**< \brief renderer context used by an engine */
    struct vox_fps_info fps_info;
    /**< \brief FPS controller info returned after each tick */
};
#endif /* VOXENGINE_SOURCE */

/**
   \brief Create voxengine.

   This function creates a lua engine. It parses command line arguments,
   initializes SDL (opens window for drawing, etc.), initializes lua state, load
   all needed modules and so on. See the main page of documentation for more
   informantion. If successful, it will put a remaining command line argument
   count in *argc, and remaining arguments in *argv.

   \param argc pointer to argument count.
   \param argv pointer to array of arguments.
   \return pointer to created engine on success or NULL.
**/
struct vox_engine* vox_create_engine (int *argc, char **argv[]);

/**
   \brief Engine tick function.

   Do an engine tick. See the main page of documentation for more
   information. This function is usually called inside an infinite loop in the
   main program.
**/
void vox_engine_tick (struct vox_engine *engine);

/**
   \brief Destroy an engine.
**/
void vox_destroy_engine (struct vox_engine *engine);

#endif
