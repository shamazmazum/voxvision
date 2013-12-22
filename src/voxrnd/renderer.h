/**
   @file renderer.h
   @brief The renderer
**/
#ifndef RENDERER_H
#define RENDERER_H

#include "../voxtrees/tree.h"
#include "renderer-ctx.h"

/**
   \brief Render a scene using renderer's context
   \param tree the scene
   \param ctx renderer's context
**/
void vox_render (struct vox_node*, vox_rnd_context*);

#endif
