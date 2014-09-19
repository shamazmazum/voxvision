/**
   @file tree.h
   @brief Things related to tree construction

   Functions for tree construction/deletion/statistical info are defined here
**/

#ifndef _TREE_H_
#define _TREE_H_

#include "params.h"

#ifdef VOXTREES_SOURCE
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
    vox_uint dots_num; /**< \brief Number of voxels in this node */
    vox_dot *dots; /**< \brief Pointer to minimal coordinates of voxels in this node */
} vox_leaf_data;

/**
   \brief Data specific to inner nodes
**/
typedef struct
{
    vox_dot center; /**< \brief Center of subdivision */
    struct vox_node *children[VOX_NS]; /**< \brief Children of this node */
} vox_inner_data;

/**
   \brief Node of a voxel octree
**/
struct vox_node
{
    vox_uint flags; /**< \brief FILL and LEAF flags */
    vox_dot bb_min; /**< \brief Minimal coordinate of the bounding box */
    vox_dot bb_max; /**< \brief Maximal coordinate of the bounding box */
    union
    {
        vox_leaf_data leaf;
        vox_inner_data inner;
    } data; /**< \brief Data specific to inner and leaf nodes */
};

/**
   \brief Align vector on voxel.

   This is a destructive operation.
   
   \param dot dot to be aligned
**/
void vox_align (vox_dot);
#else /* VOXTREES_SOURCE */
struct vox_node;
#endif /* VOXTREES_SOURCE */

/**
   \brief Turn a set of voxels into a tree.

   The Underlying set must remain valid when tree is used.

   \param set
   \param n number of voxels in the set
   \return a root node of the newly created tree
**/
struct vox_node* vox_make_tree (vox_dot[], vox_uint);

/**
   \brief Free resources used by a tree.

   Optional with GC.
**/
void vox_destroy_tree (struct vox_node*);

/**
   \brief Return number of voxels in the tree.
**/
vox_uint vox_voxels_in_tree (struct vox_node*);

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
vox_uint vox_inacc_depth (struct vox_node*, vox_uint);
float vox_inacc_balanceness (struct vox_node*);

/**
   \brief Get the bounding box for voxels in tree
**/
void vox_bounding_box (const struct vox_node*, vox_dot, vox_dot);

#endif
