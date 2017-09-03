/**
   @file renderer.h
   @brief The renderer
**/
#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "../voxtrees/tree.h"
#include "camera.h"

#ifdef VOXRND_SOURCE
typedef Uint32 square[16] __attribute__((aligned(16)));

enum vox_context_type
{
    VOX_CTX_WO_WINDOW,
    VOX_CTX_W_WINDOW
};

struct vox_rnd_ctx
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *surface;

    struct vox_node *scene;
    struct vox_camera *camera;

    square *square_output;
    int type, squares_num, ws, hs;
    float mul[3];
    float add[3];
};
#else
/**
   \brief A renderer context.
**/
struct vox_rnd_ctx
{
    SDL_Window *window;
    /**< \brief A window associated with the context, if any. **/
    SDL_Renderer *renderer;
    /**< \brief A renderer associated with the context, if any. **/
    SDL_Texture *texture;
    /**< \brief A texture associated with the context, if any. **/
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
};
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
struct vox_rnd_ctx* vox_make_context_and_window (int width, int height);

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
   \brief Free context after use
**/
void vox_destroy_context (struct vox_rnd_ctx *ctx);

/**
   \brief Render a scene on SDL surface.

   \param ctx a renderer context
**/
void vox_render (struct vox_rnd_ctx *ctx);

#endif
