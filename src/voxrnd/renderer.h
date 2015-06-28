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
   \brief Render a scene on SDL surface

   \param tree the root node for the scene
   \param camera the camera
   \param surface an SDL surface
**/
void vox_render (const struct vox_node* tree, vox_camera_interface* camera, SDL_Surface* surface);

#endif
