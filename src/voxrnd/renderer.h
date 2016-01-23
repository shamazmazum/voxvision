/**
   @file renderer.h
   @brief The renderer
**/
#ifndef RENDERER_H
#define RENDERER_H

#include <SDL/SDL.h>
#include "../voxtrees/tree.h"
#include "camera.h"

#ifdef VOXRND_SOURCE
struct vox_rnd_ctx
{
    SDL_Surface *surface;
    struct vox_node *scene;
    vox_camera_interface *camera;

    float mul[3];
    float add[3];
};
#else
struct vox_rnd_ctx;
#endif

/**
   \brief Make a renderer context.

   User must free() it after use.

   \param surface an SDL surface to render to
   \param scene the root node for the scene.
   \param camera the camera. If camera is
   NULL, you must set it later with setter
   function.

   \return a pointer to allocated context
 **/
struct vox_rnd_ctx* vox_make_renderer_context (SDL_Surface *surface, struct vox_node *scene,
                                               vox_camera_interface *camera);

/**
   \brief Scene setter for renderer context
**/
void vox_rc_set_scene (struct vox_rnd_ctx *ctx, struct vox_node *scene);

/**
   \brief Camera setter for renderer context
**/
void vox_rc_set_camera (struct vox_rnd_ctx *ctx, vox_camera_interface *camera);

/**
   \brief Render a scene on SDL surface.

   \param ctx a renderer context
**/
void vox_render (struct vox_rnd_ctx *ctx);

#endif
