#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "tree.h"
#include "geom.h"
#include "probes.h"

vox_dot vox_voxel = {1.0, 1.0, 1.0};

#ifdef STATISTICS
static void update_fill_ratio (const struct vox_box *box, size_t n) {
    float bb_volume, vox_volume;
    vox_dot size;
    unsigned int i;

    for (i=0; i<VOX_N; i++) size[i] = box->max[i] - box->min[i];
    bb_volume = size[0]*size[1]*size[2];
    vox_volume = vox_voxel[0]*vox_voxel[1]*vox_voxel[2];
    int ratio = 100*n*vox_volume/bb_volume;
    int holes = (bb_volume - n*vox_volume) / vox_volume;
    VOXTREES_FILL_RATIO (ratio);
    VOXTREES_HOLES (holes);
}
#endif

static void find_center (const vox_dot set[], size_t n, const struct vox_box *box, vox_dot res) {
    size_t i;
    memset (res, 0, sizeof(vox_dot));
    vox_dot tmp;

    for (i=0; i<n; i++) {
        vox_dot_sub (set[i], box->min, tmp);
        vox_dot_add (tmp, res, res);
    }

    for (i=0; i<VOX_N; i++) {
        res[i] = ceilf ((res[i]/n + box->min[i])/vox_voxel[i])*vox_voxel[i];
    }
}

static void calc_bounding_box (const vox_dot set[], size_t n, struct vox_box *box) {
    size_t i;
    int j;
    
    vox_dot_copy (box->min, set[0]);
    vox_dot_copy (box->max, set[0]);

    for (i=0; i<n; i++) {
        for (j=0; j<VOX_N; j++) {
            if (set[i][j] < box->min[j]) {
                box->min[j] = set[i][j];
            } else if (set[i][j] > box->max[j]) {
                box->max[j] = set[i][j];
            }
        }
    }
    vox_dot_add (box->max, vox_voxel, box->max);
}

/**
   \brief Move dots with needed subspace close to offset
   \param in a set to be modified
   \param n number of dots in set
   \param offset where to start
   \param subspace a desired subspace
   \param center the center of subdivision
   \return offset + how many dots were moved
**/
static size_t sort_set (vox_dot set[], size_t n, size_t offset, int subspace, const vox_dot center) {
    size_t i, counter = offset;
    vox_dot tmp;

    for (i=offset; i<n; i++) {
        if (get_subspace_idx (center, set[i]) == subspace) {
            vox_dot_copy (tmp, set[counter]);
            vox_dot_copy (set[counter], set[i]);
            vox_dot_copy (set[i], tmp);
            counter++;
        }
    }
    
    return counter;
}

static void vox_align (vox_dot dot) {
    unsigned int i;
    float tmp;

    for (i=0; i<VOX_N; i++) {
        tmp = floorf (dot[i] / vox_voxel[i]);
        dot[i] = vox_voxel[i] * tmp;
    }
}

static void update_bounding_box (struct vox_box *box, const vox_dot dot) {
    vox_dot dot_max;
    unsigned int i;
    vox_dot_add (dot, vox_voxel, dot_max);

    for (i=0; i<VOX_N; i++) {
        box->min[i] = (dot[i] < box->min[i]) ? dot[i] : box->min[i];
        box->max[i] = (dot_max[i] > box->max[i]) ? dot_max[i] : box->max[i];
    }
}

static void* node_alloc (int flavor) {
    /*
      FIXME: We may need to be sure if vox_dot fields of node
      structure are properly aligned in future, so use vox_alloc()
      instead of malloc()
    */
    size_t size = offsetof (struct vox_node, data);
    size_t addend;

    if (flavor & VOX_DENSE_LEAF) addend = 0;
    else if (flavor & VOX_LEAF) addend = sizeof (vox_dot*);
    else addend = sizeof (vox_inner_data);

    return vox_alloc (size+addend);
}

struct vox_node* vox_make_dense_leaf (const struct vox_box *box) {
    struct vox_node *res = node_alloc (VOX_DENSE_LEAF);
    size_t dim[3];

    vox_box_copy (&(res->bounding_box), box);
    res->flags = VOX_DENSE_LEAF;
    get_dimensions (box, dim);
    res->dots_num = dim[0]*dim[1]*dim[2];

    return res;
}

/*
  Self-explanatory. No, really.
  Being short: if number of voxels in a set is less or equal
  to maximum number allowed, create a leaf and store voxels there.
  Otherwise split the set into 2^N parts and proceed with each of subsets
  recursively.
*/
struct vox_node* vox_make_tree (vox_dot set[], size_t n) {
    struct vox_node *node  = NULL;
    int leafp;

    if (n > 0) {
        struct vox_box box;
        calc_bounding_box (set, n, &box);
        leafp = (n <= VOX_MAX_DOTS) ? VOX_LEAF : 0;
        node = node_alloc (leafp);
        node->dots_num = n;
        vox_box_copy (&(node->bounding_box), &box);
        if (leafp) {
            node->data.dots = vox_alloc (VOX_MAX_DOTS*sizeof(vox_dot));
            memcpy (node->data.dots, set, n*sizeof(vox_dot));
            node->flags = VOX_LEAF;
        } else {
            int idx;
            vox_inner_data *inner = &(node->data.inner);
            size_t new_offset, offset = 0;

            node->flags = 0;
            find_center (set, n, &(node->bounding_box), inner->center);
            for (idx=0; idx<VOX_NS; idx++) {
                new_offset = sort_set (set, n, offset, idx, inner->center);
                if (offset == 0 && new_offset == n) {
                    /*
                     * In this case my current algorithm cannot divide voxels
                     * into two or more subspaces. We just create one overflowed
                     * leaf (meaning it has number of voxels > VOX_MAX_DOTS).
                     */
                    struct vox_node *newnode = node_alloc (VOX_LEAF);
                    vox_box_copy (&(newnode->bounding_box), &(node->bounding_box));
                    /*
                     * I set VOXOVERFLOW flag only for statistics and testing.
                     * In the renderer itself overflowed leafs are processed as
                     * usual leafs.
                     */
                    newnode->flags = VOX_LEAF | VOX_OVERFLOW;
                    newnode->dots_num = n;
                    newnode->data.dots = vox_alloc (n*sizeof(vox_dot));
                    memcpy (newnode->data.dots, set, n*sizeof(vox_dot));
                    /*
                     * We can free() the old node, since it has not any
                     * allocated children.
                     */
                    free (node);
                    node = newnode;
                    break;
                }
                inner->children[idx] = vox_make_tree (set+offset, new_offset-offset);
                offset = new_offset;
            }
        }
    }
#ifdef STATISTICS
    if (!(VOX_FULLP (node))) {
        /* Empty nodes are leafs too */
        VOXTREES_EMPTY_NODE();
        VOXTREES_LEAF_NODE();
    } else {
        if (node->flags & VOX_LEAF_MASK) {
            update_fill_ratio (&(node->bounding_box), n);
            VOXTREES_LEAF_NODE();
            if (node->flags & VOX_OVERFLOW) {
                VOXTREES_LEAF_OVERFLOW();
                VOXTREES_LEAF_OVERFLOW_VOXELS(node->dots_num);
            }
        } else {
            VOXTREES_INNER_NODE();
        }
    }
#endif
    return node;
}

size_t vox_voxels_in_tree (const struct vox_node *tree) {
    return (VOX_FULLP (tree)) ? tree->dots_num : 0;
}

void vox_destroy_tree (struct vox_node *tree) {
    unsigned int i;
    if (VOX_FULLP (tree)) {
        if (tree->flags & VOX_LEAF) {
            free (tree->data.dots);
        } else if (!(tree->flags & VOX_LEAF_MASK)) {
            for (i=0; i<VOX_NS; i++) {
                vox_destroy_tree (tree->data.inner.children[i]);
            }
        }
        free (tree);
    }
}

void vox_bounding_box (const struct vox_node* tree, struct vox_box *box) {
    vox_box_copy (box, &(tree->bounding_box));
}

/*
  Turn a tree back to plain array.
  TODO: Check the tree is not frozen.
*/
static size_t flatten_tree (const struct vox_node *tree, vox_dot *set) {
    size_t count = 0;
    if (VOX_FULLP (tree)) {
        if (tree->flags & VOX_LEAF) {
            memcpy (set, tree->data.dots, tree->dots_num * sizeof(vox_dot));
            count = tree->dots_num;
        } else {
            unsigned int i;
            for (i=0; i<VOX_NS; i++) {
                size_t subcount = flatten_tree (tree->data.inner.children[i], set);
                set += subcount;
                count += subcount;
            }
        }
    }
    return count;
}

struct vox_node* vox_rebuild_tree (const struct vox_node *tree) {
    struct vox_node *new_tree = NULL;
    if (VOX_FULLP (tree)) {
        vox_dot *dots = vox_alloc (sizeof(vox_dot) * tree->dots_num);
        flatten_tree (tree, dots);
        new_tree = vox_make_tree (dots, tree->dots_num);
        free (dots);
    }
    return new_tree;
}

// TODO: Check the tree is not frozen
static int voxel_in_tree (const struct vox_node *tree, const vox_dot voxel) {
    unsigned int i;

    if (VOX_FULLP (tree) && voxel_in_box (&(tree->bounding_box), voxel)) {
        if (tree->flags & VOX_LEAF) {
            for (i=0; i<tree->dots_num; i++) {
                if (vox_dot_equalp (voxel, tree->data.dots[i])) {
                    return 1;
                }
            }
        } else {
            const vox_inner_data *inner = &(tree->data.inner);
            i = get_subspace_idx (inner->center, voxel);
            return voxel_in_tree (inner->children[i], voxel);
        }
    }
    return 0;
}

// always inserts
// TODO: Check that a tree is not frozen
static void vox_insert_voxel_ (struct vox_node **tree_ptr, const vox_dot voxel)
{
    struct vox_node *tree;
    vox_dot *dots;
    struct vox_box bb;
    struct vox_node *node;

again:
    tree = *tree_ptr;
    node = tree;

    if (VOX_FULLP (tree)) {
        vox_box_copy (&bb, &(tree->bounding_box));
        update_bounding_box (&(tree->bounding_box), voxel);
    } else {
        /*
          If tree is an empty leaf, allocate a regular leaf node and store
          the voxel there.
        */
        WITH_STAT (VOXTREES_LEAF_INSERTION());
        dots = alloca (sizeof (vox_dot) + 16);
        dots = (void*)(((unsigned long) dots + 15) & ~(unsigned long)15);
        vox_dot_copy (dots[0], voxel);
        node = vox_make_tree (dots, 1);
        goto done;
    }

    if (tree->flags & VOX_LEAF) {
        WITH_STAT (VOXTREES_LEAF_INSERTION());
        if (tree->dots_num < VOX_MAX_DOTS) {
            // We have enough space to add a voxel
            vox_dot_copy (tree->data.dots[tree->dots_num], voxel);
            tree->dots_num++;
        } else {
            // If not, create a new subtree with vox_make_tree()
            assert (tree->dots_num == VOX_MAX_DOTS);
            dots = alloca (sizeof (vox_dot)*(VOX_MAX_DOTS+1) + 16);
            dots = (void*)(((unsigned long) dots + 15) & ~(unsigned long)15);
            memcpy (dots, tree->data.dots, sizeof(vox_dot)*tree->dots_num);
            vox_dot_copy (dots[tree->dots_num], voxel);
            node = vox_make_tree (dots, tree->dots_num+1);
            vox_destroy_tree (tree);
        }
    } else {
        /*
          For inner node we need only to increment node's voxel number and
          propagate changes further.
        */
        vox_inner_data *inner = &(tree->data.inner);
        int idx = get_subspace_idx (inner->center, voxel);
        tree->dots_num++;
        tree_ptr = &(inner->children[idx]);
        goto again; // Force tail call optimization.
    }

done:
    *tree_ptr = node;
}

// TODO: Check the tree is not frozen
int vox_insert_voxel (struct vox_node **tree_ptr, vox_dot voxel) {
    int res;

    vox_align (voxel);
    res = !(voxel_in_tree (*tree_ptr, voxel));
    if (res) {
        vox_insert_voxel_ (tree_ptr, voxel);
    }
    return res;
}

int vox_insert_voxel_coord (struct vox_node **tree_ptr, float x, float y, float z) {
    vox_dot dot;
    dot[0] = x; dot[1] = y; dot[2] = z;
    return vox_insert_voxel (tree_ptr, dot);
}

// It always deletes.
// TODO: Not frozen
static void vox_delete_voxel_ (struct vox_node **tree_ptr, const vox_dot voxel)
{
    struct vox_node *tree, *node;
    unsigned int i;

again:
    tree = *tree_ptr;
    if (tree->dots_num == 1) {
        vox_destroy_tree (tree);
        *tree_ptr = NULL;
        return;
    }

    // XXX: update bounding box

    if (tree->flags & VOX_LEAF) {
        WITH_STAT (VOXTREES_LEAF_DELETION());
        for (i=0; i<tree->dots_num; i++) {
            if (vox_dot_equalp (tree->data.dots[i], voxel)) {
                break;
            }
        }
        assert (i < tree->dots_num);
        tree->dots_num--;
        memmove (tree->data.dots + i, tree->data.dots + i + 1,
                 sizeof (vox_dot) * (tree->dots_num - i));
    } else {
        // Inner node
        vox_inner_data *inner = &(tree->data.inner);
        int idx = get_subspace_idx (inner->center, voxel);
        node = inner->children[idx];
        if (node->dots_num == tree->dots_num) {
            /*
              This means that this child is the only child of its parent.
              So we can remove this parent and connect the child to parent's
              parent directly.
            */
            *tree_ptr = node;
            free (tree); // Save the child.
        } else {
            tree->dots_num--;
            tree_ptr = &(inner->children[idx]);
        }
        goto again;
    }
}

int vox_delete_voxel (struct vox_node **tree_ptr, vox_dot voxel) {
    int res;

    vox_align (voxel);
    res = voxel_in_tree (*tree_ptr, voxel);
    if (res) {
        vox_delete_voxel_ (tree_ptr, voxel);
    }
    return res;
}

int vox_delete_voxel_coord (struct vox_node **tree_ptr, float x, float y, float z) {
    vox_dot dot;
    dot[0] = x; dot[1] = y; dot[2] = z;
    return vox_delete_voxel (tree_ptr, dot);
}

void vox_dump_tree (const struct vox_node *tree) {
    const char *leaf_str = "LEAF";
    const char *dense_str = "DENSE LEAF";
    const char *inner_str = "INNER";
    const char *desc;
    unsigned int i;

    if (VOX_FULLP (tree)) {
        if (tree->flags & VOX_LEAF) {
            desc = leaf_str;
        } else if (tree->flags & VOX_DENSE_LEAF) {
            desc = dense_str;
        } else {
            desc = inner_str;
        }

        printf ("Node %p %s\n", tree, desc);
        printf ("Bounding box min <%f, %f, %f>\n",
                tree->bounding_box.min[0],
                tree->bounding_box.min[1],
                tree->bounding_box.min[2]);
        printf ("Bounding box max <%f, %f, %f>\n",
                tree->bounding_box.max[0],
                tree->bounding_box.max[1],
                tree->bounding_box.max[2]);
        printf ("Number of voxels %u\n", tree->dots_num);

        if (tree->flags & VOX_LEAF) {
            printf ("Dots:\n");
            for (i=0; i<tree->dots_num; i++) {
                printf ("<%f, %f, %f>\n",
                        tree->data.dots[i][0],
                        tree->data.dots[i][1],
                        tree->data.dots[i][2]);
            }
        } else if (!(tree->flags & VOX_LEAF_MASK)) {
            printf ("Center <%f, %f, %f>\n",
                    tree->data.inner.center[0],
                    tree->data.inner.center[1],
                    tree->data.inner.center[2]);
            printf ("Children:\n");
            for (i=0; i<VOX_NS; i++) {
                printf ("%p\n", tree->data.inner.children[i]);
            }
        }
        printf ("------Node ends here------\n");
        if (!(tree->flags & VOX_LEAF_MASK)) {
            for (i=0; i<VOX_NS; i++) {
                vox_dump_tree (tree->data.inner.children[i]);
            }
            printf ("=======Node and children end here=======\n");
        }
    }
}

void vox_set_voxel_size (float x, float y, float z) {
    vox_voxel[0] = x;
    vox_voxel[1] = y;
    vox_voxel[2] = z;
}
