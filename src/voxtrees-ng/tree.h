#ifndef TREE_H_
#define TREE_H_
#include "../voxvision.h"
#include "map.h"

#ifdef VOXTREES_NG_SOURCE
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
#else /* VOXTREES_NG_SOURCE */
struct vox_node;
#endif /* VOXTREES_NG_SOURCE */

struct vox_node* vox_make_tree (const struct vox_map_3d *map);
void vox_destroy_tree (struct vox_node *tree);
unsigned int vox_voxels_in_tree (const struct vox_node *tree);
int vox_voxel_in_tree (const struct vox_node *tree, const vox_dot voxel);
void vox_dump_tree (const struct vox_node *tree);
void vox_bounding_box (const struct vox_node *tree, struct vox_box *box);
void vox_set_voxel (const vox_dot dot);

int vox_insert_voxel (struct vox_node **tree_ptr, vox_dot voxel);
int vox_delete_voxel (struct vox_node **tree_ptr, vox_dot voxel);
struct vox_node* vox_rebuild_tree (const struct vox_node *tree);
struct vox_node* vox_make_dense_leaf (const struct vox_box *box);

extern vox_dot vox_voxel;

#endif
