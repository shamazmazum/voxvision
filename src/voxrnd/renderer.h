/**
   @file renderer.h
   @brief The renderer
**/
#ifndef RENDERER_H
#define RENDERER_H

#include <SDL/SDL.h>
#include "../voxtrees/tree.h"
#include "camera.h"

/**
   \brief Render a scene using renderer's context
   \param tree the scene
   \param ctx renderer's context
**/
void vox_render (const struct vox_node*, vox_camera_interface*, SDL_Surface*);

#endif
