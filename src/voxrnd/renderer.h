/**
   @file renderer.h
   @brief The renderer
**/
#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "../voxtrees/tree.h"
#include "camera.h"
#include "lights.h"

#ifdef VOXRND_SOURCE
typedef Uint32 square[16] __attribute__((aligned(16)));

enum vox_context_type
{
    VOX_CTX_WO_WINDOW,
    VOX_CTX_W_WINDOW
};

struct vox_draw;

struct vox_rnd_ctx
{
    SDL_Window *window;
    SDL_Surface *surface;
    struct vox_node *scene;
    struct vox_camera *camera;

    struct vox_light_manager *light_manager;
    struct vox_draw *draw; // Things needed to draw a voxel
    square *square_output;
    unsigned int squares_num, ws;

    unsigned int hs, quality;
};
#else

/**
   \brief A renderer context.
**/
struct vox_rnd_ctx
{
    SDL_Window *window;
    /**< \brief A window associated with the context, if any. **/

    SDL_Surface *surface;
    /**< \brief A surface the context will draw to.**/

    struct vox_node *scene;
    /**< \brief A scene associated with the context.

       To set this value, use vox_context_set_scene() rather than writing to
       this field directly.
    **/

    struct vox_camera *camera;
    /**< \brief A camera associated with the context.

       To set this value, use vox_context_set_camera() rather than writing to
       this field directly.
    **/

    struct vox_light_manager *light_manager;
    /**< \brief A light manager for the context

       This light manager created automatically when the context is created. So
       you do not need to free it manually.
    **/
};
#endif

/**
   \brief Best rendering quality.

   This quality setting forces the renderer to perform a full search
   for every rendered pixel starting from root of the tree.
**/
#define VOX_QUALITY_BEST 0b00

/**
   \brief Fast rendering, middle quality.

   This quality setting enables the renderer to reuse the tree leafs
   from a previous search to find an intersection of a ray from the
   camera and the tree. This may cause artifacts on edges of rendered
   objects.
**/
#define VOX_QUALITY_FAST 0b01

/**
   \brief Average rendering speed, good quality.

   This settings tells the renderer to choose between
   `VOX_QUALITY_FAST` and `VOX_QUALITY_BEST` settings for each block
   of 4x4 pixels.
**/
#define VOX_QUALITY_ADAPTIVE 0b10

/**
   \brief Enable ray merging mode.

   This mode allows a ray which belongs to every second column on the screen to
   be merged with a ray from every first if this ray travels a long distance
   from the origin. This mode works ONLY in conjunction with adaptive mode.
**/
#define VOX_QUALITY_RAY_MERGE 0b0100

/**
   \brief Accurate ray merging mode.

   This mode is like standard ray merging mode, but it disables ray merging on
   edges of objects.
**/
#define VOX_QUALITY_RAY_MERGE_ACCURATE 0b1000

/**
   \brief Ray merging mode mask.
**/
#define VOX_QUALITY_RM_MASK 0b1100 /* RM stands for Ray Merge */

/**
   \brief Rendering mode mask.
**/
#define VOX_QUALITY_MODE_MASK 0b11

#ifdef VOXRND_SOURCE
#define VOX_QUALITY_RM_MAX 0b1000
#define VOX_QUALITY_MODE_MAX 0b10
#endif

/**
   \brief Make a renderer context from SDL surface.

   This function creates a renderer context based upon earlier created SDL
   surface. vox_render() will render frames to that surface. User must free it
   with vox_destroy_context() after use. The underlying surface will not be
   freed.

   NB: Surface width must be multiple of 16 and surface heigth must be multiple
   of 4. If this does not hold, function silently returns NULL.
**/
struct vox_rnd_ctx* vox_make_context_from_surface (SDL_Surface *surface);

/**
   \brief Create a window and attach context to it.

   This function creates a window and a context associated with it. vox_render()
   will render frames to an internal surface which can be shown on window with
   vox_redraw(). User must free it with vox_destroy_context() after use.

   \param width Width of the window.
   \param height Height of the window.
   \return Context or NULL in case of SDL error.

   NB: width must be multiple of 16 and heigth must be multiple
   of 4. If this does not hold, function silently returns NULL.
**/
struct vox_rnd_ctx* vox_make_context_and_window (unsigned int width, unsigned int height);

/**
   \brief Redraw a window associated with this context.

   This function will redraw window after call to vox_render() if context has a
   window.
**/
void vox_redraw (struct vox_rnd_ctx *ctx);

/**
   \brief Scene setter for renderer context
**/
void vox_context_set_scene (struct vox_rnd_ctx *ctx, struct vox_node *scene);

/**
   \brief Camera setter for renderer context
**/
void vox_context_set_camera (struct vox_rnd_ctx *ctx, struct vox_camera *camera);

/**
   \brief Set quality of the renderer

   \param ctx The renderer's context.
   \param quality Currently one of `QUALITY_BEST`, `QUALITY_FAST` or
          `QUALITY_ADAPTIVE`.
   \return 1 on success, 0 otherwise. Function call is unsuccessfull when
          `quality` parameter is wrong.
**/
int vox_context_set_quality (struct vox_rnd_ctx *ctx, unsigned int quality);

/**
   \brief Free context after use
**/
void vox_destroy_context (struct vox_rnd_ctx *ctx);

/**
   \brief Render a scene on SDL surface.

   \param ctx a renderer context
**/
void vox_render (struct vox_rnd_ctx *ctx);

#endif
