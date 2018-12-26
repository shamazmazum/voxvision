/**
   @file engine.h
   @brief Lua engine
**/
#ifndef ENGINE_H
#define ENGINE_H
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
    dispatch_queue_t rendering_queue;
    unsigned int width, height;
    unsigned int flags;
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
   \brief Tells voxengine to start in debug mode.
**/
#define VOX_ENGINE_DEBUG 1

/**
   \brief Create voxengine.

   This function creates a lua engine: initializes SDL, opens a window,
   initializes lua environment, loads needed modules and lua control script and
   so on. See the main page of documentation for more informantion. If
   you wish to pass an array of strings to lua control script, they
   will be visible in `arg` table having they indices starting from 1.

   \param width Width of the window.
   \param height Height of the window.
   \param flags Currently 0 for normal work or `VOX_ENGINE_DEBUG` for
          debug mode.
   \param script Control script in lua
   \param nargs Number of arguments
   \param arguments An array of zero terminated strings which you want
          to pass to lua as arguments. Must be NULL if nargs is
          zero. As this function returns, the array is no longer
          needed by the engine.
   \return Pointer to created engine on success or NULL.
**/
struct vox_engine* vox_create_engine (unsigned int width, unsigned int height,
                                      unsigned int flags,
                                      const char *script,
                                      int nargs, char * const arguments[]);

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
