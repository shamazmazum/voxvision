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
#ifndef SSE_INTRIN
    int unused;
#endif
    struct vox_node *children[VOX_NS]; /**< \brief Children of this node */
} vox_inner_data;

/**
   \brief Node of a voxel octree
**/
struct vox_node
{
    vox_dot bb_min; /**< \brief Minimal coordinate of the bounding box */
    vox_dot bb_max; /**< \brief Maximal coordinate of the bounding box */
    size_t dots_num;
#ifdef SSE_INTRIN
    int unused[2];
#endif
    union
    {
        vox_dot *dots;
        vox_inner_data inner;
    } data; /**< \brief Data specific to inner and leaf nodes */
};
#else /* VOXTREES_SOURCE */
struct vox_node;
#endif /* VOXTREES_SOURCE */

/**
   \brief Turn a set of voxels into a tree.

   The underlying set must remain valid when tree is used.

   \param set a set of dots (of type vox_dot) to form a tree
   \param n number of voxels in the set
   \return a root node of the newly created tree
**/
struct vox_node* vox_make_tree (vox_dot set[], size_t n);

/**
   \brief Free resources used by a tree.

   Note, that you must free underlying set by yourself.
**/
void vox_destroy_tree (struct vox_node *tree);

/**
   \brief Return number of voxels in the tree.
**/
size_t vox_voxels_in_tree (struct vox_node *tree);

/**
   \brief Calculate a depth of the tree.
   
   This function is called inaccurate because
   it uses a predefined path from root to leaf,
   treating all other paths to any leaf having
   the same length

   \param tree the tree
   \return depth of the tree
**/
int vox_inacc_depth (struct vox_node *tree);

/**
   \brief Estimate balanceness of the tree.

   If value returned from this function is far from 1.0,
   the tree is unbalanced
**/
float vox_inacc_balanceness (struct vox_node *tree);

/**
   \brief Get the bounding box for voxels in tree
**/
void vox_bounding_box (const struct vox_node *tree, vox_dot min, vox_dot max);

#endif
