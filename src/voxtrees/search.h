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
   \param leaf where the leaf node is stored, if leaf is not NULL
   \return 1 if intersection is found, 0 otherwise
**/
int vox_ray_tree_intersection (const struct vox_node* tree, const vox_dot origin, const vox_dot dir,
                               vox_dot res, const struct vox_node** leaf);

/**
   \brief Find out if a ball collides with voxels in a tree

   \param tree the tree
   \param center center of the ball
   \param radius radius of the ball
   \return 1 if collision was found, 0 otherwise
**/
int vox_tree_ball_collidep (struct vox_node* tree, const vox_dot center, float radius);

#endif
