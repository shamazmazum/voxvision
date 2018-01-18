#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "geom.h"
#include "probes.h"

#define HOLLOW 0
#define SOLID 1
#define LEAF_MAX_VOXELS 7

vox_dot vox_voxel = {1,1,1};

void vox_set_voxel (const vox_dot dot)
{
    memcpy (vox_voxel, dot, sizeof (vox_voxel));
}

static void check_indices (const unsigned int dim[], const struct vox_box *box)
{
    int a = 0;
    int b = 0;
    int i;

    for (i=0; i<3; i++) {
        a += box->min[i] <= box->max[i];
        b += box->max[i] <= dim[i] * vox_voxel[i];
    }

    assert (a == 3 && b == 3);
}

static unsigned int count_voxels (const struct vox_map_3d *map,
                                  const struct vox_box *box)
{
    unsigned int count = 0;
    size_t idx;
    float i,j,k;

    check_indices (map->dim, box);

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

    check_indices (map->dim, box);
    vox_dot_set (center, 0, 0, 0);

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

    int l;
    for (l=0; l<3; l++)
        center[l] = vox_voxel[l] * ceilf (center[l] / count / vox_voxel[l]);
}

static int bounding_box (const struct vox_map_3d *map,
                         int which,
                         struct vox_box *bounding_box,
                         const struct vox_box *box)
{
    float i,j,k;
    size_t idx;
    int found = 0, value;
    vox_dot tmp;
    int l;

    check_indices (map->dim, box);

    for (i=box->min[0]; i<box->max[0]; i+=vox_voxel[0]) {
        for (j=box->min[1]; j<box->max[1]; j+=vox_voxel[1]) {
            for (k=box->min[2]; k<box->max[2]; k+=vox_voxel[2]) {
                idx = map->dim[1]*map->dim[2]*i +
                    map->dim[2]*j + k;
                value = !!(map->map[idx]);

                if (value == which) {
                    if (!found) {
                        found = 1;
                        vox_dot_set (bounding_box->min, i, j, k);
                        vox_dot_copy (bounding_box->max, bounding_box->min);
                    } else {
                        vox_dot_set (tmp, i, j, k);
                        for (l=0; l<3; l++) {
                            bounding_box->min[l] = fminf (tmp[l], bounding_box->min[l]);
                            bounding_box->max[l] = fmaxf (tmp[l], bounding_box->max[l]);
                        }
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
    check_indices (map->dim, box);

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
                    vox_dot_set (dots[n], i, j, k);
                    n++;
                }
            }
        }
    }
}

static struct vox_node* make_tree (const struct vox_map_3d *map, const struct vox_box *box)
{
    WITH_STAT (VOXTREES_NG_INC_REC_LEVEL());
    struct vox_node *node = NULL;
    struct vox_box actual_bb;
    int has_bb = bounding_box (map, SOLID, &actual_bb, box);
    if (!has_bb) goto done;
    unsigned int voxel_num = count_voxels (map, &actual_bb);
    assert (voxel_num > 0);
    unsigned int volume = box_volume (&actual_bb);
    unsigned int hole_num = volume - voxel_num;
    /*
     * Choose what we want to store in the node: solid voxels or
     * holes.
     */
    unsigned int data_num = (hole_num < voxel_num)? hole_num: voxel_num;
    assert (data_num >= 0);
    node = aligned_alloc (16, sizeof (struct vox_node));
    node->flags = 0;

    /*
     * Set actual bounding box which is a bounding box for SOLID
     * voxels which belong to this node.
     */
    vox_box_copy (&(node->actual_bb), &actual_bb);

    /*
     * Special case: voxel_num != 0 && data_num == 0, in other words
     * we have voxels belonging to the node, but do not have any data
     * to store.
     */
    if (data_num == 0) {
        node->flags = NODATA;
        node->leaf_data.dots_num = 0;
        goto done;
    }

    /*
     * Set data bounding box. It is a bounding box for data being
     * stored (e.g. "holes" or "solid" voxels).
     */
    if (data_num == voxel_num) {
        vox_box_copy (&(node->data_bb), &(node->actual_bb));
    } else {  /* We store holes instead of voxels */
        node->flags |= CONTAINS_HOLES;
        bounding_box (map, HOLLOW, &(node->data_bb), &(node->actual_bb));
        if (box_inside_box (&(node->actual_bb), &(node->data_bb), 1)) node->flags |= COVERED;
    }

    if (data_num <= LEAF_MAX_VOXELS) {
        /* Create a leaf. */
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

struct vox_node* vox_make_tree (const struct vox_map_3d *map)
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
    unsigned int num, volume, i;
    if (tree == NULL) return count;

    if (tree->flags & LEAF) {
        num = tree->leaf_data.dots_num;
        if (tree->flags & CONTAINS_HOLES) {
            volume = box_volume (&(tree->actual_bb));
            count = volume - num;
        } else count = num;
    } else {
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

void vox_dump_tree (const struct vox_node *tree)
{
    printf ("==== Node %p ====\n", tree);
    if (tree != NULL) {
        int flags = tree->flags;
        int nodata = TREE_NODATA_P (tree);
        printf ("%s, contains %s, %s, %s\n",
                (flags & LEAF)? "Leaf": "Inner node",
                (flags & CONTAINS_HOLES)? "holes": "solid voxels",
                (flags & COVERED)? "covered": "not covered",
                (nodata)? "has no data": "has data");
        printf ("Actual bounding box: <%f, %f, %f> - <%f, %f, %f>\n",
                tree->actual_bb.min[0],
                tree->actual_bb.min[1],
                tree->actual_bb.min[2],
                tree->actual_bb.max[0],
                tree->actual_bb.max[1],
                tree->actual_bb.max[2]);
        if (!nodata) {
            int i;
            printf ("Data bounding box: <%f, %f, %f> - <%f, %f, %f>\n",
                    tree->data_bb.min[0],
                    tree->data_bb.min[1],
                    tree->data_bb.min[2],
                    tree->data_bb.max[0],
                    tree->data_bb.max[1],
                    tree->data_bb.max[2]);
            if (flags & LEAF) {
                printf ("Leaf data:\n");
                for (i=0; i<tree->leaf_data.dots_num; i++)
                    printf ("<%f, %f, %f> ",
                            tree->leaf_data.dots[i][0],
                            tree->leaf_data.dots[i][1],
                            tree->leaf_data.dots[i][2]);
                printf ("\n===============\n");
            } else {
                printf ("Center of division: <%f, %f, %f>\n",
                        tree->inner_data.center[0],
                        tree->inner_data.center[1],
                        tree->inner_data.center[2]);
                printf ("Node children:\n");
                for (i=0; i<8; i++)
                    printf ("%p ", tree->inner_data.children[i]);
                printf ("\n===============\n");
                for (i=0; i<8; i++)
                    vox_dump_tree (tree->inner_data.children[i]);
            }
        }
    }
}

void vox_bounding_box (const struct vox_node *tree, struct vox_box *box)
{
    vox_box_copy (box, &(tree->actual_bb));
}
