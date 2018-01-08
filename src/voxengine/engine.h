/**
   @file engine.h
   @brief Lua engine
**/
#ifndef ENGINE_H
#define ENGINE_H
#include <SDL2/SDL.h>
#ifdef USE_GCD
#include <dispatch/dispatch.h>
#else
#include "../gcd-stubs.c"
#endif

#include "../voxtrees/tree.h"
#include "../voxrnd/camera.h"
#include "../voxrnd/fps-control.h"
#include "../voxrnd/renderer.h"
#include "../voxrnd/cd.h"

#ifdef VOXENGINE_SOURCE
struct vox_engine {
    struct vox_rnd_ctx *ctx;
    lua_State *L;

    struct vox_cd *cd;
    dispatch_queue_t rendering_queue;

    int width, height;
};
#else /* VOXENGINE_SOURCE */

/**
   \brief Voxengine instance.

   This is a part of voxengine structure visible to user. You must create
   voxengine with vox_create_engine.
**/

struct vox_engine {
    struct vox_rnd_ctx *ctx;
    /**< \brief renderer context used by an engine */
};
#endif /* VOXENGINE_SOURCE */

/**
   \brief Create voxengine.

   This function creates a lua engine: initializes SDL, opens a window,
   initializes lua environment, loads needed modules and lua control script and
   so on. See the main page of documentation for more informantion.

   \param width Width of the window.
   \param height Height of the window.
   \param script Control script in lua
   \return Pointer to created engine on success or NULL.
**/
struct vox_engine* vox_create_engine (int width, int height, const char *script);

/**
   \brief Return type for `vox_engine_tick`
**/
typedef int vox_engine_status;

/**
   \brief Return true if quit was requested from the control script.
**/
#define vox_engine_quit_requested(stat) (!(stat))

/**
   \brief Engine tick function.

   Do an engine tick. See the main page of documentation for more
   information. This function is usually called inside an infinite loop in the
   main program.

   \return Engine status (e.g. quit requested).
**/
vox_engine_status vox_engine_tick (struct vox_engine *engine);

/**
   \brief Destroy an engine.
**/
void vox_destroy_engine (struct vox_engine *engine);

#endif
