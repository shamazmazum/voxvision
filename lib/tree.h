#ifndef _TREE_H_
#define _TREE_H_

#include <stdint.h>
#include "params.h"

#define LEAF 0
#define FULL 1

/**
   Is the node a leaf?
**/
#define LEAFP(node) ((node)->flags & (1<<LEAF))

/**
   Is the node full?
**/
#define FULLP(node) ((node)->flags & (1<<FULL))

/**
   Data specific to leaf nodes
**/
typedef struct
{
    unsigned int dots_num; /**< Number of voxels in this node */
    float dots[MAX_DOTS][N]; /**< Minimal coordinates of voxels in this node */
} leaf_data;

/**
   Data specific to inner nodes
**/
typedef struct
{
    float center[N]; /**< Center of subdivision */
    struct node *children[NS]; /**< Children of this node */
} inner_data;

/**
   Data specific to inner and leaf nodes
**/
union node_data
{
    leaf_data leaf;
    inner_data inner;
};

/**
   Node of a voxel octree
**/
struct node
{
    unsigned int flags; /**< Fill and leaf flags */
    float bb_min[N]; /**< Minimal coordinate of the bounding box */
    float bb_max[N]; /**< Maximal coordinate of the bounding box */
    union node_data data; /**< Data specific to inner and leaf nodes */
};

float* sum_vector (const float*, const float*, float*);
uint8_t get_subspace_idx (const float*, const float*);
struct node* make_tree (float [][N], int);
int voxels_in_tree (struct node*);
int inacc_depth (struct node*, int);
float inacc_balanceness (struct node*);
//void destroy_tree (struct node*);

#endif
