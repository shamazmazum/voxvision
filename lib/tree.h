/**
   @file tree.h
   @brief Things related to tree construction

   Functions for tree construction/deletion/statistical info are defined here
**/

#ifndef _TREE_H_
#define _TREE_H_

#include "params.h"

#define VOX_LEAF 0
#define VOX_FULL 1

/**
   Is the node a leaf?
**/
#define VOX_LEAFP(node) ((node)->flags & (1<<VOX_LEAF))

/**
   Is the node full?
**/
#define VOX_FULLP(node) ((node)->flags & (1<<VOX_FULL))

/**
   \brief Data specific to leaf nodes
**/
typedef struct
{
    unsigned int dots_num; /**< Number of voxels in this node */
    float (*dots)[VOX_N]; /**< Pointer to minimal coordinates of voxels in this node */
} vox_leaf_data;

/**
   \brief Data specific to inner nodes
**/
typedef struct
{
    float center[VOX_N]; /**< Center of subdivision */
    struct vox_node *children[VOX_NS]; /**< Children of this node */
} vox_inner_data;

/**
   \brief Data specific to inner and leaf nodes
**/
union vox_node_data
{
    vox_leaf_data leaf;
    vox_inner_data inner;
};

/**
   \brief Node of a voxel octree
**/
struct vox_node
{
    unsigned int flags; /**< Fill and leaf flags */
    float bb_min[VOX_N]; /**< Minimal coordinate of the bounding box */
    float bb_max[VOX_N]; /**< Maximal coordinate of the bounding box */
    union vox_node_data data; /**< Data specific to inner and leaf nodes */
};

/**
   \brief Align vector on voxel
   \return passed argument
**/
float* vox_align (float*);

/**
   \brief Turn a set of voxels into a tree.
   \return a root node of the newly created tree
**/
struct vox_node* vox_make_tree (float [][VOX_N], unsigned int);

/**
   \brief Free resources used by a tree.

   Optional with GC.
**/
void vox_destory_tree (struct vox_node*);

/**
   \brief Return number of voxels in the tree.
**/
unsigned int vox_voxels_in_tree (struct vox_node*);

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
unsigned int vox_inacc_depth (struct vox_node*, unsigned int);
float vox_inacc_balanceness (struct vox_node*);

#endif
