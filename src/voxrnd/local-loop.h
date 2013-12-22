/**
   @file local-loop.h
   @brief Renderer's local loop
**/
#ifndef LOCALITY_HELPER_H
#define LOCALITY_HELPER_H

#include "../voxtrees/tree.h"
#include "renderer-ctx.h"

/**
   \brief Run renderer's local loop

   This function detects intersection of a tree and a probing ray
   operating on renderer's context. On each iteration the ray is
   changed using increment function. If intersection is found a
   specified action is performed.

   \param tree a tree
   \param n number of iterations to be performed
   \param action function which accepts renderer's context as its
          first and only argument. Called when intersection is found
   \param inc function which is used to change context (the probing
          ray, for example) on each iteration.
   \param ctx renderer's context
**/
void vox_local_loop (struct vox_node*, int, void (*) (vox_rnd_context*), \
                     void (*) (vox_rnd_context*), vox_rnd_context*);

#endif
