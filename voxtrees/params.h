/**
   @file params.h
   @brief Global parameters used by the library
**/

#ifndef _PARAMS_H_
#define _PARAMS_H_

#include "params_var.h"

// Macros

/**
   \brief Number of dimensions.

   With SSE support must be 0<VOX_N<=4.
**/
#define VOX_N 3

/**
   Two in power of VOX_N.
   Number of subspaces.
**/
#define VOX_NS 8

/**
   Maximum number of dots
   in tree leaf.
**/
#define VOX_MAX_DOTS 7

/**
   Maximal depth of tree.
   Used only for path saver
**/
#define VOX_MAX_DEPTH 15

/**
   Maximal depth between 2 nodes
   which still considered local.
   Must be < VOX_MAX_DEPTH
**/
#define VOX_MAX_DEPTH_LOCAL 3

// Types

typedef struct vox_node* vox_tree_path[VOX_MAX_DEPTH];
typedef unsigned int vox_uint;
#if defined(SSE_ENABLE_CONS) || defined(SSE_ENABLE_SEARCH)
typedef float vox_dot[4] __attribute__ ((aligned (16)));
#else
typedef float vox_dot[VOX_N];
#endif

// Global vars

/**
   Sides of voxel.
**/
extern vox_dot vox_voxel;

/**
   Level of details (LOD)
**/
extern vox_uint vox_lod;

#endif
