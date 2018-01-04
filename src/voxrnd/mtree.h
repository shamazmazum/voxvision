#ifndef __MTREE_H_
#define __MTREE_H_
#include "../voxvision.h"

struct vox_sphere {
    vox_dot center;
    float radius;
};

#ifdef VOXRND_SOURCE
// It's here only for tests
#define MAX_CHILDREN 3 // > 2

struct vox_mtree_node {
    struct vox_sphere bounding_sphere;
    struct vox_mtree_node *parent;
    unsigned int leaf;
    unsigned int num;
    union {
        // Leave room for extra one which will cause a split
        struct vox_mtree_node *children[MAX_CHILDREN+1];
        struct vox_sphere *spheres; // Pointer to an array of MAX_CHILDREN+1 spheres
    } data;
};
#else
struct vox_mtree_node;
#endif

void vox_mtree_add_sphere (struct vox_mtree_node **nodeptr, const struct vox_sphere *s);
void vox_mtree_destroy (struct vox_mtree_node *node);
void vox_mtree_dump (const struct vox_mtree_node *node);
unsigned int vox_mtree_items (const struct vox_mtree_node *node);

#endif
