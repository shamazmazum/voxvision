/**
   @file mtree.h
   @brief Support for M-trees

   I decided to expose the M-tree related header to user, but it is not useful for
   anything now.
**/
#ifndef __MTREE_H_
#define __MTREE_H_
#include "../voxvision.h"

/**
   \brief Sphere structure.

   Spheres are objects which are held in M-tree.
**/
struct vox_sphere {
    vox_dot center;
    /**< \brief Center of the sphere. **/

    vox_dot color;
    /**<
       \brief Color of the sphere.

       It would be better to hold arbitrary userdata here. But for now color is the only
       useful "userdata" I can use. So I hardcoded this type in declaration of this
       structure.
    **/

    float radius;
    /**< \brief Radius of the sphere. **/

    float sqr_radius;
    /**<
       \brief Square of the sphere's radius.

       This field is set by voxrnd library and can be completely ignored by user.
    **/
};

#ifdef VOXTREES_SOURCE
// It's here only for tests
//#define MTREE_MAX_CHILDREN 3 // > 2
#define MTREE_MAX_CHILDREN 9 // 128 bytes in memory

struct vox_mtree_node {
    struct vox_sphere bounding_sphere;
    struct vox_mtree_node *parent;
    unsigned int leaf;
    unsigned int num;
    union {
        // Leave room for extra one which will cause a split
        struct vox_mtree_node *children[MTREE_MAX_CHILDREN+1];
        struct vox_sphere *spheres; // Pointer to an array of MAX_CHILDREN+1 spheres
    } data;
};
#else
struct vox_mtree_node;
#endif

/**
   \brief Add a sphere to an M-tree.

   Inserts a sphere to an M-tree. If the new root is created for that tree, it will be set
   to a place `nodeptr` points to. Note that implementation cannot handle cases with
   different spheres having the same center.

   \param nodeptr Pointer to a root node
   \param s Sphere to be inserted. Once sphere is added to a tree, it is no more needed by
          the library, so you can reclaim space used by it (or allocate sphere on stack).
   \return 1 On success (sphere was not in a tree) or 0 otherwise.
**/
int vox_mtree_add_sphere (struct vox_mtree_node **nodeptr, const struct vox_sphere *s);

/**
   \brief Remove sphere from a tree.

   Removes a sphere from a tree, possibly leaving the tree unbalanced. If the resulting
   tree is empty, `NULL` is stored to a place `nodeptr` points to.

   \param nodeptr Pointer to a root node
   \param s Sphere to be inserted. Once sphere is added to a tree, it is no more needed by
          the library, so you can reclaim space used by it (or allocate sphere on stack).
   \return 1 On success (sphere was in a tree) or 0 otherwise.
**/
int vox_mtree_remove_sphere (struct vox_mtree_node **nodeptr, const struct vox_sphere *s);

/**
   \brief Destroy a tree, freeing all used space.
**/
void vox_mtree_destroy (struct vox_mtree_node *node);

/**
   \brief Dump a tree to stdout.

   Use carefully as it may produce very long output.
**/
void vox_mtree_dump (const struct vox_mtree_node *node);

/**
   \brief Get a number of items (spheres) in M-tree.
**/
unsigned int vox_mtree_items (const struct vox_mtree_node *node);

/**
   \brief Check if a sphere is in a tree.

   Note that only `center` and `radius` fields are used for the check.

   \return A leaf of the tree in which sphere is contained.
**/
const struct vox_mtree_node*
vox_mtree_contains_sphere (const struct vox_mtree_node *node,
                           const struct vox_sphere *s);

/**
   \brief Do a job for all spheres which contain a specific dot.

   \param node A pointer to an M-tree root node.
   \param dot A dot for which search is performed.
   \param block A piece of work to do given as a C block. This block accepts a sphere to
   which the dot belongs to.
**/
void vox_mtree_spheres_containing (const struct vox_mtree_node *node, const vox_dot dot,
                                   void (^block)(const struct vox_sphere *s));

/**
   \brief Callback-styled version of `vox_mtree_spheres_containing()`.

   In this version, instead of a block, a callback must be supplied. The callback accepts
   a sphere as the first argument (with the same meaning as for
   `vox_mtree_spheres_containing()`). Also it accepts a second argument, `arg` which can
   hold arbitrary user-defined data.

   \param node A pointer to an M-tree root node.
   \param dot A dot for which search is performed.
   \param callback A piece of work to do given as a callback.
   \param thunk Some arbitrary userdata which is passed to the callback as the second
          argument, unchanged.
**/
void vox_mtree_spheres_containing_f (const struct vox_mtree_node *node, const vox_dot dot,
                                     void (*callback)(const struct vox_sphere *s, void *arg),
                                     void *thunk);
#endif
