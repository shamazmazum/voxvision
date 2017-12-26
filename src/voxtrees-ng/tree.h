#ifndef TREE_H_
#define TREE_H_
#include "types.h"

#define LEAF 1
#define CONTAINS_HOLES 2
#define COVERED 4
#define NODATA (8+4+2+1)
#define TREE_NODATA_P(tree) ((tree->flags & NODATA) == NODATA)

struct inner_node {
    vox_dot center;
    struct vox_node *children[8];
};

struct leaf {
    unsigned int dots_num;
    vox_dot *dots;
};

struct vox_node {
    struct vox_box actual_bb;
    struct vox_box data_bb;
    int flags;
    union {
        struct inner_node inner_data;
        struct leaf leaf_data;
    };
};

struct vox_node* vox_make_tree (const struct vox_map_3d *map);
void vox_destroy_tree (struct vox_node *tree);
unsigned int vox_voxels_in_tree (const struct vox_node *tree);
int vox_voxel_in_tree (const struct vox_node *tree, const vox_dot voxel);
void vox_dump_tree (const struct vox_node *tree);
void vox_bounding_box (const struct vox_node *tree, struct vox_box *box);

extern vox_dot vox_voxel;

#endif
