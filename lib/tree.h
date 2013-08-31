#ifndef _TREE_H_
#define _TREE_H_

#include <stdint.h>
#include "params.h"

/**
   Is the node a leaf?
**/
#define LEAFP(node) !((node)->children)

/**
   Node of a voxel octree
**/
struct node
{
    float bb_min[N]; /**< Minimal coordinate of the bounding box */
    float bb_max[N]; /**< Maximal coordinate of the bounding box */

    struct node **children; /**< Children of this node (if any) */
    
    uint8_t dots_num; /**< Number of voxels in the node (or 0 if not a leaf) */
    float dots[MAX_DOTS][N]; /**<  Minimal coordinates of voxels in this node
                                (or the central dot of subdivision at dots[0] if not a leaf */
};


float* sum_vector (const float*, const float*, float*);
uint8_t get_subspace_idx (const float*, const float*);
struct node* make_tree (float [][N], int);
int voxels_in_tree (struct node*);
int inacc_depth (struct node*, int);
float inacc_balanceness (struct node*);
void destroy_tree (struct node*);

#endif
