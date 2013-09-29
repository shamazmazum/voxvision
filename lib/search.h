/**
   @file search.h
   @brief Algorithms for searching in tree

   Searching for intersection with a ray and a tree and (maybe) other
**/

#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "tree.h"

struct tagged_coord
{
    int tag;
    float coord[N];
};

/**
   \brief Find intersection of a tree and a ray.
   
   \param tree a tree
   \param origin starting point of the ray
   \param dir direction of the ray
   \param res where result is stored
   \param depth an initial depth of recursion. Better to specify 1
   \param lod Level of Detail. depth = 1, lod = 0 means no level of detail
   \return 1 if intersection was found, 0 otherwise
**/
int ray_tree_intersection (struct node*, const float*, const float*, float*, int, int);
#endif
