/**
   @file search.h
   @brief Algorithms for searching in tree

   Searching for intersection with a ray and a tree and (maybe) other
**/

#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "tree.h"

/**
   \brief Find intersection of a tree and a ray.
   
   \param tree a tree
   \param origin starting point of the ray
   \param dir direction of the ray
   \param res where result is stored
   \param depth an initial depth of recursion. Must be 1
   \param lod Level of Detail. lod = 0 means no level of detail
   \param path where the path to found voxel will be stored
   \return number of elements in path if intersection was found, 0 otherwise
**/
int ray_tree_intersection (struct node*, const float*, const float*, float*, unsigned int, unsigned int, tree_path);

/**
   \brief Returns non-zero value if a ball collides with voxels in a tree
   \param tree the tree
   \param center center of the ball
   \param radius radius of the ball
   \return 1 if collision was found, 0 otherwise
**/
int tree_ball_collisionp (struct node*, const float*, float);
#endif
