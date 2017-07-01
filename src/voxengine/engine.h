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

    lua_State *L;
    struct vox_cd *cd;

    int width, height, script_executed;
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
};
#endif /* VOXENGINE_SOURCE */

/**
   \brief Create voxengine.

   This function creates a lua engine: initializes SDL, opens a window,
   initializes lua environment, loads needed modules and so on. See the main
   page of documentation for more informantion.

   \param width Width of the window.
   \param height Height of the window.
   \return pointer to created engine on success or NULL.
**/
struct vox_engine* vox_create_engine (int width, int height);

/**
   \brief Load lua script.

   This function loads lua control script, unloading the previous one, if such
   script exists. FIXME: Panics and quits on failure.

   \param engine An initialized engine
   \param script Script file name
**/
void vox_engine_load_script (struct vox_engine *engine, const char *script);

/**
   \brief Engine tick function.

   Do an engine tick. See the main page of documentation for more
   information. This function is usually called inside an infinite loop in the
   main program.

   \return 1 if tick is performed, 0 otherwise (e.g. script is not loaded,
   nothing to do).
**/
int vox_engine_tick (struct vox_engine *engine);

/**
   \brief Is quit was requested?

   This function checks if `request_quit()` was called in lua script.

   \return 1 if quit was requested, 0 otherwise.
**/
int vox_engine_quit_requested (struct vox_engine *engine);

/**
   \brief Destroy an engine.
**/
void vox_destroy_engine (struct vox_engine *engine);

#endif
