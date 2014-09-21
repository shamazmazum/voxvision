/**
   @file search.h
   @brief Algorithms for searching in tree

   Searching for intersection with a ray and a tree and (maybe) other
**/

#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "tree.h"

#ifdef VOXTREES_SOURCE
struct vox_search_state
{
    vox_uint depth, maxdepth;
    const struct vox_node *tree;
    vox_tree_path path;
};
#else
struct vox_search_state;
#endif

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
vox_uint vox_ray_tree_intersection (const struct vox_node*, const vox_dot, const vox_dot, vox_dot, vox_uint, vox_tree_path);

/**
   \brief Returns non-zero value if a ball collides with voxels in a tree
   \param tree the tree
   \param center center of the ball
   \param radius radius of the ball
   \return 1 if collision was found, 0 otherwise
**/
int vox_tree_ball_collidep (struct vox_node*, const vox_dot, float);

struct vox_search_state* vox_make_search_state (const struct vox_node *tree);
vox_uint vox_ray_tree_intersection_wstate (struct vox_search_state*, const vox_dot, const vox_dot, vox_dot);
#endif
