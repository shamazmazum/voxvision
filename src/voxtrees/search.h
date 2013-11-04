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
   \param path where the path to found voxel will be stored
   \return number of elements in path if intersection was found, 0 otherwise
**/
vox_uint vox_ray_tree_intersection (struct vox_node*, const vox_dot, const vox_dot, vox_dot, vox_uint, vox_tree_path);

/**
   \brief Find intersection of a tree and local rays using data locality.
   
   Does not prove that data has local properties so can give wrong intersections, when used incorrectly.
   Must be used for bunch of rays after call to vox_ray_tree_intersection for the key ray.

   \param path a path returned by vox_ray_tree_intersection for the key ray
   \param origin starting point of the ray
   \param dir direction of the ray
   \param res where result is stored
   \param depth an initial depth of recursion. Must be 1 for the first call
   \param n returned value of vox_ray_tree_intersection. Number of elements in path.
   \return new depth value best suited for future calls
**/
vox_uint vox_local_rays_tree_intersection (const vox_tree_path, const vox_dot, const vox_dot, vox_dot, vox_uint, vox_uint);

/**
   \brief Returns non-zero value if a ball collides with voxels in a tree
   \param tree the tree
   \param center center of the ball
   \param radius radius of the ball
   \return 1 if collision was found, 0 otherwise
**/
int vox_tree_ball_collidep (struct vox_node*, const vox_dot, float);
#endif
