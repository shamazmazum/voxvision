/**
   @file tree.h
   @brief Things related to tree construction

   Functions for tree construction/deletion/statistical info are defined here
**/

#ifndef _TREE_H_
#define _TREE_H_

#include "params.h"

#ifdef VOXTREES_SOURCE

#define LEAF 1
#define DENSE_LEAF 2
#define VOX_LEAFP(node) (!(node) || ((node)->flags & (LEAF|DENSE_LEAF)))
#define VOX_DENSE_LEAFP(node) (!(node) || ((node)->flags & DENSE_LEAF))

#define VOX_FULLP(node) ((node))

typedef struct
{
    vox_dot center; /**< \brief Center of subdivision */
#ifndef SSE_INTRIN
    int unused;
#endif
    struct vox_node *children[VOX_NS]; /**< \brief Children of this node */
} vox_inner_data;

struct vox_node
{
    struct vox_box bounding_box;
    unsigned int flags;
    unsigned int dots_num;
#ifdef SSE_INTRIN
    int unused[2];
#endif
    union
    {
        vox_dot *dots;
        vox_inner_data inner;
    } data;
};
#else /* VOXTREES_SOURCE */
struct vox_node;
#endif /* VOXTREES_SOURCE */

/**
   \brief Turn a set of voxels into a tree.

   The underlying set must remain valid while tree is used.

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
   \brief Get the bounding box for voxels in the tree
**/
void vox_bounding_box (const struct vox_node *tree, struct vox_box *box);

/**
   \brief Optimize an underlying set of the tree.

   This function replaces an old underlying set with a new one
   which possibly is smaller than the old one. You can free the
   old set after this operation. The new set must be valid while
   the tree is used. You can free() it after destruction of the
   tree.

   \return A pointer to the new set.
**/
vox_dot* vox_recopy_tree (struct vox_node *tree);

#endif
