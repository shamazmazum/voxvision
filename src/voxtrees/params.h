/**
   @file params.h
   @brief Global parameters used by the library
**/

#ifndef _PARAMS_H_
#define _PARAMS_H_

#include "../voxvision.h"

// Macros
#ifdef VOXTREES_SOURCE

/**
   \brief Number of dimentions
**/
#define VOX_N 3

/**
   \brief Must be 2 in power of VOX_N.
   
   Number of subspaces.
**/
#define VOX_NS 8

/**
   \brief Maximum number of dots in tree leaf.
**/
#define VOX_MAX_DOTS 7
#endif /* VOXTREES_SOURCE */

// Types

typedef unsigned int vox_uint;

// Global vars

/**
   \brief Sides of voxel.
**/
extern vox_dot vox_voxel;

/**
   \brief Level of details (LOD)
**/
extern vox_uint vox_lod;

#endif
