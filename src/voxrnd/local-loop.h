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
   changed using iter_post function. If intersection is found a
   specified action is performed.

   \param tree a tree
   \param action function which accepts renderer's context as its
          first and only argument. Called when intersection is found
   \param iter_post function which is used to change context (the
          probing ray, for example) after each iteration. Its
          arguments are the context and the number of current
          iteration. It is also responsible for telling vox_local_loop
          when it has to stop. If iter_post returns 0, no more
          iterations will be performed.
   \param ctx renderer's context
**/
void vox_local_loop (struct vox_node*, void (*) (vox_rnd_context*), \
                     int (*) (vox_rnd_context*, int), vox_rnd_context*);

#endif
