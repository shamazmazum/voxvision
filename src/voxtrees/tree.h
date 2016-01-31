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
#define LEAF_MASK 3
#define VOX_LEAFP(node) (!(node) || ((node)->flags & LEAF_MASK))
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

   The underlying set is destructively modified and can be freed after
   creation.

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
size_t vox_voxels_in_tree (const struct vox_node *tree);

/**
   \brief Get the bounding box for voxels in the tree
**/
void vox_bounding_box (const struct vox_node *tree, struct vox_box *box);

/**
   \brief Rebuild a tree.

   You can rebuild a tree completely and destroy the old one with
   vox_destroy_tree(). The new tree can be more balanced. Use this
   after a big amount of insertions.
**/
struct vox_node* vox_rebuild_tree (const struct vox_node *tree);

/**
   \brief Insert a voxel in the tree on the fly.

   Many applications of this function will result in unbalanced tree.
   You can rebalance the tree by recreating it with vox_rebuild_tree()

   \return 1 on success, 0 if the voxel was already in the tree.
**/
int vox_insert_voxel (struct vox_node **tree_ptr, vox_dot voxel);

/**
   \brief Delete a voxel from the tree on the fly.

   You can call vox_rebuild_tree() after many applications of this
   function to get a more balanced tree.

   \return 1 on success, 0 if there was no such voxel in the tree.
**/
int vox_delete_voxel (struct vox_node **tree_ptr, vox_dot voxel);

/**
   \brief Dump a tree to standard output stream.

   This is ment solely for debugging purposes and can produce
   very big output.
**/
void vox_dump_tree (const struct vox_node *tree);

#endif
