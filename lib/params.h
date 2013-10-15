/**
   @file params.h
   @brief Global parameters used by the library
**/

#ifndef _PARAMS_H_
#define _PARAMS_H_

/**
   Number of dimensions.
**/
#define N 3

/**
   Two in power of N.
   Number of subspaces.
**/
#define NS 8

/**
   Maximum number of dots
   in tree leaf.
**/
#define MAX_DOTS 7

/**
   Maximal depth of tree.
   Used only for path saver
**/
#define MAX_DEPTH 15

/**
   Maximal depth between 2 nodes
   which still considered local.
   Must be < MAX_DEPTH
**/
#define MAX_DEPTH_LOCAL 3

typedef struct node* tree_path[MAX_DEPTH];

/**
   Sides of voxel.
**/
extern float voxel[N];

#endif
