/**
   @file tree.h
   @brief Things related to tree construction

   Functions for tree construction/deletion/statistical info are defined here
**/

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

/**
   \brief Take sum of two vectors.
   
   \param res an array where the result is stored
   \return Third passed argument, whichs contains the sum
**/
float* sum_vector (const float*, const float*, float*);

/**
   \brief Calculate a subspace index for the dot.
   
   For N-dimentional space we have 2^N ways to place the dot
   around the center of subdivision. Find which way is the case.
   
   \param dot1 the center of subdivision
   \param dot2 the dot we must calculate index for
   \return The subspace index in the range [0,2^N-1]
**/
uint8_t get_subspace_idx (const float*, const float*);

/**
   \brief Turn a set of voxels into a tree.
   \return a root node of the newly created tree
**/
struct node* make_tree (float [][N], int);

/**
   \brief Return number of voxels in the tree.
**/
int voxels_in_tree (struct node*);

/**
   \brief Calculate a depth of the tree.
   
   This function is called inaccurate because
   it uses a predefined path from root to leaf,
   treating all other paths to any leaf having
   the same length

   \param tree the tree
   \param res an initial value of depth
   \return res + actual depth
**/
int inacc_depth (struct node*, int);
float inacc_balanceness (struct node*);
//void destroy_tree (struct node*);

#endif
