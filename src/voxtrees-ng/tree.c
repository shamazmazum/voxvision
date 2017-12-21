#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "tree.h"
#include "geom.h"
#ifdef STATISTICS
#include <voxtrees-ng-dtrace.h>
#endif

#define HOLLOW 0
#define SOLID 1
#define LEAF_MAX_VOXELS 7

vox_dot vox_voxel = {1,1,1};

static unsigned int count_voxels (const struct vox_map_3d *map,
                                  const struct vox_box *box)
{
    unsigned int count = 0;
    size_t idx;
    float i,j,k;

    assert (box->min[0] <= map->dim[0]*vox_voxel[0] ||
            box->min[1] <= map->dim[1]*vox_voxel[1] ||
            box->min[2] <= map->dim[2]*vox_voxel[2]);

    assert (box->max[0] <= map->dim[0]*vox_voxel[0] ||
            box->max[1] <= map->dim[1]*vox_voxel[1] ||
            box->max[2] <= map->dim[2]*vox_voxel[2]);

    assert (box->min[0] <= box->max[0] ||
            box->min[1] <= box->max[1] ||
            box->min[2] <= box->max[2]);
    
    for (i=box->min[0]; i<box->max[0]; i+=vox_voxel[0]) {
        for (j=box->min[1]; j<box->max[1]; j+=vox_voxel[1]) {
            for (k=box->min[2]; k<box->max[2]; k+=vox_voxel[2]) {
                idx = map->dim[1]*map->dim[2]*i +
                    map->dim[2]*j + k;
                count += !!map->map[idx];
            }
        }
    }

    return count;
}

static void find_center (const struct vox_map_3d *map,
                         int which, vox_dot center,
                         const struct vox_box *box)
{
    float i,j,k, count = 0;
    size_t idx;
    int value;
    memset (center, 0, sizeof (vox_dot));
    assert (box->min[0] <= map->dim[0]*vox_voxel[0] ||
            box->min[1] <= map->dim[1]*vox_voxel[1] ||
            box->min[2] <= map->dim[2]*vox_voxel[2]);

    assert (box->max[0] <= map->dim[0]*vox_voxel[0] ||
            box->max[1] <= map->dim[1]*vox_voxel[1] ||
            box->max[2] <= map->dim[2]*vox_voxel[2]);

    assert (box->min[0] <= box->max[0] ||
            box->min[1] <= box->max[1] ||
            box->min[2] <= box->max[2]);

    for (i=box->min[0]; i<box->max[0]; i+=vox_voxel[0]) {
        for (j=box->min[1]; j<box->max[1]; j+=vox_voxel[1]) {
            for (k=box->min[2]; k<box->max[2]; k+=vox_voxel[2]) {
                idx = map->dim[1]*map->dim[2]*i +
                    map->dim[2]*j + k;
                value = !!map->map[idx];
                if (value == which) {
                    vox_dot tmp = {i, j, k};
                    vox_dot_add (tmp, center, center);
                    count++;
                }
            }
        }
    }
    center[0] = vox_voxel[0] * ceilf (center[0] / count / vox_voxel[0]);
    center[1] = vox_voxel[1] * ceilf (center[1] / count / vox_voxel[1]);
    center[2] = vox_voxel[2] * ceilf (center[2] / count / vox_voxel[2]);
}

static int bounding_box (const struct vox_map_3d *map,
                         int which,
                         struct vox_box *bounding_box,
                         const struct vox_box *box)
{
    float i,j,k;
    size_t idx;
    int found = 0, value;
    assert (box->min[0] <= map->dim[0]*vox_voxel[0] ||
            box->min[1] <= map->dim[1]*vox_voxel[1] ||
            box->min[2] <= map->dim[2]*vox_voxel[2]);

    assert (box->max[0] <= map->dim[0]*vox_voxel[0] ||
            box->max[1] <= map->dim[1]*vox_voxel[1] ||
            box->max[2] <= map->dim[2]*vox_voxel[2]);
    
    assert (box->min[0] <= box->max[0] ||
            box->min[1] <= box->max[1] ||
            box->min[2] <= box->max[2]);

    for (i=box->min[0]; i<box->max[0]; i+=vox_voxel[0]) {
        for (j=box->min[1]; j<box->max[1]; j+=vox_voxel[1]) {
            for (k=box->min[2]; k<box->max[2]; k+=vox_voxel[2]) {
                idx = map->dim[1]*map->dim[2]*i +
                    map->dim[2]*j + k;
                value = !!(map->map[idx]);

                if (value == which) {
                    if (!found) {
                        found = 1;
                        bounding_box->min[0] = i; bounding_box->max[0] = i;
                        bounding_box->min[1] = j; bounding_box->max[1] = j;
                        bounding_box->min[2] = k; bounding_box->max[2] = k;
                    } else {
                        bounding_box->min[0] = (i < bounding_box->min[0])? i: bounding_box->min[0];
                        bounding_box->min[1] = (j < bounding_box->min[1])? j: bounding_box->min[1];
                        bounding_box->min[2] = (k < bounding_box->min[2])? k: bounding_box->min[2];

                        bounding_box->max[0] = (i > bounding_box->max[0])? i: bounding_box->max[0];
                        bounding_box->max[1] = (j > bounding_box->max[1])? j: bounding_box->max[1];
                        bounding_box->max[2] = (k > bounding_box->max[2])? k: bounding_box->max[2];
                    }
                }
            }
        }
    }

    if (found)
        vox_dot_add (bounding_box->max, vox_voxel, bounding_box->max);
    return found;
}

static unsigned int box_volume (const struct vox_box *box)
{
    vox_dot tmp;
    vox_dot_sub (box->max, box->min, tmp);
    float volume = tmp[0]*tmp[1]*tmp[2]/(vox_voxel[0]*vox_voxel[1]*vox_voxel[2]);
    assert ((unsigned int) volume == volume);
    return (unsigned int) volume;
}

static void collect_data (const struct vox_map_3d *map, int which, vox_dot *dots,
                          int num, const struct vox_box *box)
{
    assert (box->min[0] <= map->dim[0]*vox_voxel[0] ||
            box->min[1] <= map->dim[1]*vox_voxel[1] ||
            box->min[2] <= map->dim[2]*vox_voxel[2]);

    assert (box->max[0] <= map->dim[0]*vox_voxel[0] ||
            box->max[1] <= map->dim[1]*vox_voxel[1] ||
            box->max[2] <= map->dim[2]*vox_voxel[2]);

    assert (box->min[0] <= box->max[0] ||
            box->min[1] <= box->max[1] ||
            box->min[2] <= box->max[2]);

    float i,j,k;
    size_t idx;
    int value;
    int n = 0;
    for (i=box->min[0]; i<box->max[0]; i+=vox_voxel[0]) {
        for (j=box->min[1]; j<box->max[1]; j+=vox_voxel[1]) {
            for (k=box->min[2]; k<box->max[2]; k+=vox_voxel[2]) {
                idx = map->dim[1]*map->dim[2]*i +
                    map->dim[2]*j + k;
                value = !!(map->map[idx]);
                if (value == which) {
                    assert (n < num);
                    dots[n][0] = i;
                    dots[n][1] = j;
                    dots[n][2] = k;
                    n++;
                }
            }
        }
    }
}

static struct vox_node* make_tree (struct vox_map_3d *map, const struct vox_box *box)
{
#ifdef STATISTICS
    VOXTREES_NG_INC_REC_LEVEL();
#endif
    struct vox_node *node = NULL;
    struct vox_box actual_bb;
    int has_bb = bounding_box (map, SOLID, &actual_bb, box);
    if (!has_bb) goto done;
    unsigned int voxel_num = count_voxels (map, &actual_bb);
    assert (voxel_num > 0);
    unsigned int volume = box_volume (&actual_bb);
    unsigned int hole_num = volume - voxel_num;
    unsigned int data_num = (hole_num < voxel_num)? hole_num: voxel_num;
    assert (data_num >= 0);
    node = aligned_alloc (16, sizeof (struct vox_node));
    node->flags = 0;
    // Setting actual bounding box
    vox_box_copy (&(node->actual_bb), &actual_bb);

    // Special case: voxel_num != 0 && data_num == 0
    if (data_num == 0) {
        node->flags = NODATA;
        node->leaf_data.dots_num = 0;
        goto done;
    }

    // Set data bounding box
    if (data_num == voxel_num) {
        vox_box_copy (&(node->data_bb), &(node->actual_bb));
    } else {  // We store holes instead of voxels
        node->flags |= CONTAINS_HOLES;
        bounding_box (map, HOLLOW, &(node->data_bb), &(node->actual_bb));
        if (box_inside_box (&(node->actual_bb), &(node->data_bb), 1)) node->flags |= COVERED;
    }

    if (data_num <= LEAF_MAX_VOXELS) {
        node->flags |= LEAF;
        node->leaf_data.dots_num = data_num;
        node->leaf_data.dots = aligned_alloc (16, sizeof (vox_dot)*data_num);
        collect_data (map, data_num == voxel_num, node->leaf_data.dots, data_num, &(node->data_bb));
    } else {
        int i;
        find_center (map, data_num == voxel_num, node->inner_data.center, &(node->data_bb));
        assert (dot_inside_box (&(node->data_bb), node->inner_data.center, 0));

        for (i=0; i<8; i++) {
            struct vox_box subspace;
            subspace_box (&(node->data_bb), node->inner_data.center, &subspace, i);
            node->inner_data.children[i] = make_tree (map, &subspace);
        }
    }

done:
#ifdef STATISTICS
    if (node == NULL) {
        /* Empty nodes are leafs too */
        VOXTREES_NG_EMPTY_NODE();
        VOXTREES_NG_LEAF_NODE();
    } else {
        if (node->flags & CONTAINS_HOLES) VOXTREES_NG_CONTAINS_HOLES();
        if (node->flags & COVERED) VOXTREES_NG_COVERED();

        if (node->flags & LEAF) {
        VOXTREES_NG_LEAF_NODE();
        VOXTREES_NG_FILL_RATIO (100 * data_num / volume);
        } else VOXTREES_NG_INNER_NODE();
    }
    VOXTREES_NG_DEC_REC_LEVEL();
#endif
    return node;
}

struct vox_node* vox_make_tree (struct vox_map_3d *map)
{
    struct vox_box box = {
        .min = {0, 0, 0},
        .max = {map->dim[0]*vox_voxel[0],
                map->dim[1]*vox_voxel[1],
                map->dim[2]*vox_voxel[2]}
    };
    return make_tree (map, &box);
}

void vox_destroy_tree (struct vox_node *tree)
{
    if (tree != NULL) {
        if ((tree->flags & LEAF) && ((tree->flags & NODATA) != NODATA))
            free (tree->leaf_data.dots);
        else {
            int i;
            for (i=0; i<8; i++) vox_destroy_tree (tree->inner_data.children[i]);
        }
        free (tree);
    }
}

unsigned int vox_voxels_in_tree (const struct vox_node *tree)
{
    unsigned int count = 0;
    if (tree == NULL) return count;

    if (tree->flags & LEAF) {
        unsigned int num = tree->leaf_data.dots_num;
        if (tree->flags & CONTAINS_HOLES) {
            unsigned int volume = box_volume (&(tree->actual_bb));
            count = volume - num;
        } else count = num;
    } else {
        int i;
        count = box_volume (&(tree->actual_bb)) - box_volume (&(tree->data_bb));
        for (i=0; i<8; i++)
            count += vox_voxels_in_tree (tree->inner_data.children[i]);
    }
    return count;
}

int vox_voxel_in_tree (const struct vox_node *tree, const vox_dot voxel)
{
    if (tree == NULL) return 0;
    if (!voxel_inside_box (&(tree->actual_bb), voxel, 0)) return 0;
    if (TREE_NODATA_P (tree)) return 1;

    int i, inside;
    if (tree->flags & LEAF) {
        if ((tree->flags & CONTAINS_HOLES) &&
            !voxel_inside_box (&(tree->data_bb), voxel, 0))
            return 1;
        int equals = 0;
        for (i=0; i<tree->leaf_data.dots_num; i++) {
            equals = vox_dot_equalp (voxel, tree->leaf_data.dots[i]);
            if (equals) break;
        }
        if (tree->flags & CONTAINS_HOLES) inside = !equals;
        else inside = equals;
    } else {
        int idx = subspace_idx (tree->inner_data.center, voxel);
        if (tree->flags & CONTAINS_HOLES) {
            inside = !voxel_inside_box (&(tree->data_bb), voxel, 0) ||
                vox_voxel_in_tree (tree->inner_data.children[idx], voxel);
        } else {
            inside = vox_voxel_in_tree (tree->inner_data.children[idx], voxel);
        }
    }

    return inside;
}
