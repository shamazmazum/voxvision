/**
   @file local-loop.h
   @brief Renderer's local loop
**/
#ifndef LOCALITY_HELPER_H
#define LOCALITY_HELPER_H

#include "../voxtrees/tree.h"
#include "renderer-ctx.h"

#define VOX_LL_ADAPTIVE 1
#define VOX_LL_FIXED 2
#define VOX_LL_MAXITER(n) ((n)<<4)

#define VOX_LL_GET_ITER(n) ((n)>>4)

/**
   \brief Run renderer's local loop

   This function detects intersection of a tree and a probing ray
   operating on renderer's context. On each iteration the ray is
   changed using increment function. If intersection is found a
   specified action is performed.

   \param tree a tree
   \param action function which accepts renderer's context as its
          first and only argument. Called when intersection is found
   \param inc function which is used to change context (the probing
          ray, for example) on each iteration. Accepts context and the
          iteration number as arguments
   \param ctx renderer's context
   \param mode working mode. Must be any of supported modes ORed with
          VOX_LL_MAXITER (maximum number of iterations)
   \return Number of iterations performed
**/
int vox_local_loop (struct vox_node*, void (*) (vox_rnd_context*),     \
                     void (*) (vox_rnd_context*, int), vox_rnd_context*, int);

#endif
