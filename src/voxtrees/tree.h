/**
   @file tree.h
   @brief Things related to tree construction

   Functions for tree construction/deletion/statistical info are defined here
**/

#ifndef _TREE_H_
#define _TREE_H_

#include "params.h"

#ifdef VOXTREES_SOURCE

/**
   Is the node a leaf?
**/
#define VOX_LEAFP(node) (!(node) || ((node)->dots_num <= VOX_MAX_DOTS))

/**
   Is the node full?
**/
#define VOX_FULLP(node) ((node))

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
    vox_dot bb_min; /**< \brief Minimal coordinate of the bounding box */
    vox_dot bb_max; /**< \brief Maximal coordinate of the bounding box */
    vox_uint dots_num;
    int unused1;
#ifdef SSE_INTRIN
    int unused2[2];
#endif
    union
    {
        vox_dot *dots;
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
   \return depth of the tree
**/
vox_uint vox_inacc_depth (struct vox_node* tree);
float vox_inacc_balanceness (struct vox_node*);

/**
   \brief Get the bounding box for voxels in tree
**/
void vox_bounding_box (const struct vox_node*, vox_dot, vox_dot);

#endif
