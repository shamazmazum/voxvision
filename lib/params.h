/**
   @file params.h
   @brief Global parameters used by the library
**/

#ifndef _PARAMS_H_
#define _PARAMS_H_

// Macros

/**
   Number of dimensions.
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
typedef float vox_dot[VOX_N];

// Global vars

/**
   Sides of voxel.
**/
extern float vox_voxel[VOX_N];

/**
   Level of details (LOD)
**/
extern vox_uint vox_lod;

#endif
