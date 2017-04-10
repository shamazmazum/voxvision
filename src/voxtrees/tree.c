#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "tree.h"
#include "geom.h"
#include "statistics.h"

vox_dot vox_voxel = {1.0, 1.0, 1.0};

#ifdef SSE_INTRIN
static void find_center (const vox_dot set[], size_t n, vox_dot res)
{
    size_t i;
    __v4sf len = _mm_set_ps1 (n);
    __v4sf sum = _mm_set_ps1 (0.0);
    __v4sf voxel = _mm_load_ps (vox_voxel);

    for (i=0; i<n; i++) sum += _mm_load_ps (set[i]);
    sum /= len;

    /*
      Align the center of division, so any voxel belongs to only one subspace
      entirely. Faces of voxels may be the exception though
    */
    __v4sf resv = sum / voxel;
    resv = _mm_ceil_ps (resv) * voxel;

    _mm_store_ps (res, resv);
}

static void calc_bounding_box (const vox_dot set[], size_t n, struct vox_box *box)
{
    size_t i;
    __v4sf min = _mm_load_ps (set[0]);
    __v4sf max = min;

    for (i=1; i<n; i++)
    {
        min = _mm_min_ps (min, _mm_load_ps (set[i]));
        max = _mm_max_ps (max, _mm_load_ps (set[i]));
    }
    max += _mm_load_ps (vox_voxel);

    _mm_store_ps (box->min, min);
    _mm_store_ps (box->max, max);
}

/*
  SIMD version of this function. For use in SIMD code.
*/
static int get_subspace_idx_simd (__v4sf center, __v4sf dot)
{
    __v4sf sub = dot - center;
    return (_mm_movemask_ps (sub) & 0x07);
}

static size_t sort_set (vox_dot set[], size_t n, size_t offset, int subspace, const vox_dot center)
{
    size_t i, counter = offset;
    __v4sf center_, boundary_dot, dot;
    center_ = _mm_load_ps (center);
    boundary_dot = _mm_load_ps (set[offset]);

    for (i=offset; i<n; i++)
    {
        dot = _mm_load_ps (set[i]);
        if (get_subspace_idx_simd (center_, dot) == subspace)
        {
            _mm_store_ps (set[counter], dot);
            _mm_store_ps (set[i], boundary_dot);
            boundary_dot = _mm_load_ps (set[++counter]);
        }
    }

    return counter;
}

static void vox_align (vox_dot dot)
{
    __v4sf d = _mm_load_ps (dot);
    __v4sf voxel = _mm_load_ps (vox_voxel);
    __v4sf tmp = _mm_floor_ps (d / voxel);
    _mm_store_ps (dot, tmp*voxel);
}

static void update_bounding_box (struct vox_box *box, const vox_dot dot)
{
    __v4sf d = _mm_load_ps (dot);
    __v4sf d_max = d + _mm_load_ps (vox_voxel);

    __v4sf box_min = _mm_min_ps (d, _mm_load_ps (box->min));
    __v4sf box_max = _mm_max_ps (d_max, _mm_load_ps (box->max));

    _mm_store_ps (box->min, box_min);
    _mm_store_ps (box->max, box_max);
}

#else /* SSE_INTRIN */
static void find_center (const vox_dot set[], size_t n, vox_dot res)
{
    size_t i;
    memset (res, 0, sizeof(vox_dot));

    for (i=0; i<n; i++) vox_sum_vector (set[i], res, res);
    for (i=0; i<VOX_N; i++) res[i] = ceilf (res[i]/n/vox_voxel[i])*vox_voxel[i];
}

static void calc_bounding_box (const vox_dot set[], size_t n, struct vox_box *box)
{
    size_t i;
    int j;
    
    vox_dot_copy (box->min, set[0]);
    vox_dot_copy (box->max, set[0]);

    for (i=0; i<n; i++)
    {
        for (j=0; j<VOX_N; j++)
        {
            if (set[i][j] < box->min[j]) box->min[j] = set[i][j];
            else if (set[i][j] > box->max[j]) box->max[j] = set[i][j];
        }
    }
    vox_sum_vector (box->max, vox_voxel, box->max);
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
static size_t sort_set (vox_dot set[], size_t n, size_t offset, int subspace, const vox_dot center)
{
    size_t i, counter = offset;
    vox_dot tmp;

    for (i=offset; i<n; i++)
    {
        if (get_subspace_idx (center, set[i]) == subspace)
        {
            vox_dot_copy (tmp, set[counter]);
            vox_dot_copy (set[counter], set[i]);
            vox_dot_copy (set[i], tmp);
            counter++;
        }
    }
    
    return counter;
}

static void vox_align (vox_dot dot)
{
    int i;
    float tmp;

    for (i=0; i<VOX_N; i++)
    {
        tmp = floorf (dot[i] / vox_voxel[i]);
        dot[i] = vox_voxel[i] * tmp;
    }
}

static void update_bounding_box (struct vox_box *box, const vox_dot dot)
{
    vox_dot dot_max;
    int i;
    vox_sum_vector (dot, vox_voxel, dot_max);

    for (i=0; i<VOX_N; i++)
    {
        box->min[i] = (dot[i] < box->min[i]) ? dot[i] : box->min[i];
        box->max[i] = (dot_max[i] > box->max[i]) ? dot_max[i] : box->max[i];
    }
}

#endif /* SSE_INTRIN */

static void* node_alloc (int flavor)
{
    /*
      FIXME: We may need to be sure if vox_dot fields of node
      structure are properly aligned in future, so use aligned_alloc()
      instead of malloc()
    */
    size_t size = offsetof (struct vox_node, data);
    size_t addend;

    if (flavor & DENSE_LEAF) addend = 0;
    else if (flavor & LEAF) addend = sizeof (vox_dot*);
    else addend = sizeof (vox_inner_data);

    return aligned_alloc (16, size+addend);
}

WITH_STAT (static int recursion = -1;)

struct vox_node* vox_make_dense_leaf (const struct vox_box *box)
{
    struct vox_node *res = node_alloc (DENSE_LEAF);
    size_t dim[3];

    vox_box_copy (&(res->bounding_box), box);
    res->flags = DENSE_LEAF;
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
struct vox_node* vox_make_tree (vox_dot set[], size_t n)
{
    struct vox_node *node  = NULL;
    int leafp, densep;
    WITH_STAT (recursion++);

    if (n > 0)
    {
        struct vox_box box;
        calc_bounding_box (set, n, &box);
        densep = (dense_set_p (&box, n)) ? DENSE_LEAF : 0;
        leafp = (n <= VOX_MAX_DOTS) ? LEAF : 0;
        node = node_alloc (densep | leafp);
        node->dots_num = n;
        vox_box_copy (&(node->bounding_box), &box);
        if (densep) node->flags = DENSE_LEAF;
        else if (leafp)
        {
            node->data.dots = aligned_alloc (16, VOX_MAX_DOTS*sizeof(vox_dot));
            memcpy (node->data.dots, set, n*sizeof(vox_dot));
            node->flags = LEAF;
        }
        else
        {
            int idx;
            vox_inner_data *inner = &(node->data.inner);
            size_t new_offset, offset = 0;

            node->flags = 0;
            find_center (set, n, inner->center);
            for (idx=0; idx<VOX_NS; idx++)
            {
                /*
                  XXX: Current algorithm always divides voxels by two or more
                  subspaces. Is it true in general?
                */
                new_offset = sort_set (set, n, offset, idx, inner->center);
                assert (new_offset - offset != n);
                inner->children[idx] = vox_make_tree (set+offset, new_offset-offset);
                offset = new_offset;
            }
        }
    }
#ifdef STATISTICS
    if (!(VOX_FULLP (node)))
    {
        /* Empty nodes are leafs too */
        gstats.empty_nodes++;
        gstats.leaf_nodes++;
        gstats.depth_hist[recursion]++;
    }
    else
    {
        if (node->flags & LEAF_MASK)
        {
            if (node->flags & DENSE_LEAF)
            {
                gstats.dense_leafs++;
                gstats.dense_dots+=n;
            }
            else update_fill_ratio_hist (&(node->bounding_box), n);
            gstats.leaf_nodes++;
            gstats.depth_hist[recursion]++;
        }
        else gstats.inner_nodes++;
    }
#endif
    WITH_STAT (recursion--);
    return node;
}

size_t vox_voxels_in_tree (const struct vox_node *tree)
{
    return (VOX_FULLP (tree)) ? tree->dots_num : 0;
}

void vox_destroy_tree (struct vox_node *tree)
{
    int i;
    if (VOX_FULLP (tree))
    {
        if (tree->flags & LEAF)
            free (tree->data.dots);
        else if (!(tree->flags & LEAF_MASK))
            for (i=0; i<VOX_NS; i++) vox_destroy_tree (tree->data.inner.children[i]);
        free (tree);
    }
}

void vox_bounding_box (const struct vox_node* tree, struct vox_box *box)
{
    vox_box_copy (box, &(tree->bounding_box));
}

/*
  Turn a tree back to plain array
*/
static size_t flatten_tree (const struct vox_node *tree, vox_dot *set)
{
    size_t count = 0;
    if (VOX_FULLP (tree))
    {
        if (tree->flags & LEAF)
        {
            memcpy (set, tree->data.dots, tree->dots_num * sizeof(vox_dot));
            count = tree->dots_num;
        }
        else if (tree->flags & DENSE_LEAF)
        {
            int i,j,k;
            vox_dot current;
            size_t dim[VOX_N];
            /*
              XXX: This requires reworking.

              Here I find the the number of voxels in a dense leaf along each
              axis. This number is, of course, integer, but I have only
              floating point coordinates of the bounding box. Therefore dim[]
              will be calculated improperly if the precision of floating
              point arithmetic is not enough.
            */
            get_dimensions (&(tree->bounding_box), dim);
            assert (dim[0]*dim[1]*dim[2] == tree->dots_num);
            current[0] = tree->bounding_box.min[0];
            for (i=0; i<dim[0]; i++)
            {
                current[1] = tree->bounding_box.min[1];
                for (j=0; j<dim[1]; j++)
                {
                    current[2] = tree->bounding_box.min[2];
                    for (k=0; k<dim[2]; k++)
                    {
                        vox_dot_copy (set[count], current);
                        current[2] += vox_voxel[2];
                        count++;
                    }
                    current[1] += vox_voxel[1];
                }
                current[0] += vox_voxel[0];
            }
        }
        else
        {
            int i;
            for (i=0; i<VOX_NS; i++)
            {
                size_t subcount = flatten_tree (tree->data.inner.children[i], set);
                set += subcount;
                count += subcount;
            }
        }
    }
    return count;
}

struct vox_node* vox_rebuild_tree (const struct vox_node *tree)
{
    struct vox_node *new_tree = NULL;
    if (VOX_FULLP (tree))
    {
        vox_dot *dots = aligned_alloc (16, sizeof(vox_dot) * tree->dots_num);
        flatten_tree (tree, dots);
        new_tree = vox_make_tree (dots, tree->dots_num);
        free (dots);
    }
    return new_tree;
}

static int voxel_in_tree (const struct vox_node *tree, const vox_dot voxel)
{
    int i;

    if (VOX_FULLP (tree) &&
        voxel_in_box (&(tree->bounding_box), voxel))
    {
        if (tree->flags & DENSE_LEAF) return 1;
        if (tree->flags & LEAF)
        {
            for (i=0; i<tree->dots_num; i++)
                if (vox_dot_equalp (voxel, tree->data.dots[i])) return 1;
        }
        else
        {
            const vox_inner_data *inner = &(tree->data.inner);
            i = get_subspace_idx (inner->center, voxel);
            return voxel_in_tree (inner->children[i], voxel);
        }
    }
    return 0;
}

static void vox_insert_voxel_ (struct vox_node **tree_ptr, const vox_dot voxel);
static struct vox_node* __attribute__((noinline)) // always inserts
    insert_in_big_dense (struct vox_node *tree, const vox_dot voxel)
{
    /*
      Put a dense leaf and this voxel in a new inner node.
      Benchmarks show that this is the slowest part of this function
      (especially when compiled with SSE_INTRIN).
    */
    int idx1, idx2;
    struct vox_node *node;
    vox_inner_data *inner;

    node = node_alloc (0);
    inner = &(node->data.inner);
    memset (inner, 0, sizeof (vox_inner_data));
    vox_box_copy (&(node->bounding_box), &(tree->bounding_box));
    update_bounding_box (&(node->bounding_box), voxel);
    node->flags = 0;
    node->dots_num = tree->dots_num + 1;
    // Can we find another center of division easily?
    closest_vertex (&(tree->bounding_box), voxel, inner->center);
#ifdef SSE_INTRIN
    __v4sf inner_vector = _mm_load_ps (tree->bounding_box.min) +
        _mm_load_ps (tree->bounding_box.max);
    inner_vector /= _mm_set_ps1 (2.0);
    __v4sf center = _mm_load_ps (inner->center);
    idx1 = get_subspace_idx_simd (center, inner_vector);
    idx2 = get_subspace_idx_simd (center, _mm_load_ps(voxel));
#else
    int i;
    vox_dot inner_dot;
    for (i=0; i<VOX_N; i++)
        inner_dot[i] = (tree->bounding_box.min[i]+tree->bounding_box.max[i]) / 2;
    idx1 = get_subspace_idx (inner->center, inner_dot);
    idx2 = get_subspace_idx (inner->center, voxel);
#endif
    assert (idx1 != idx2);
    inner->children[idx1] = tree;
    // Insert voxel in an empty leaf
    vox_insert_voxel_ (&(inner->children[idx2]), voxel);

    return node;
}

// always inserts
static void vox_insert_voxel_ (struct vox_node **tree_ptr, const vox_dot voxel)
{
    struct vox_node *tree;
    vox_dot *dots;
    struct vox_box bb;
    struct vox_node *node;

again:
    tree = *tree_ptr;
    node = tree;

    /*
      Aggregate voxels in the dense leaf if possible.
      Also update node's bounding box, only if this node
      is not a dense one.
    */
    if (VOX_FULLP (tree))
    {
        vox_box_copy (&bb, &(tree->bounding_box));
        update_bounding_box (&(tree->bounding_box), voxel);
        if (dense_set_p (&(tree->bounding_box), tree->dots_num+1))
        {
            if (tree->flags & DENSE_LEAF) tree->dots_num++;
            else
            {
                node = vox_make_dense_leaf (&(tree->bounding_box));
                assert (node->dots_num == tree->dots_num+1);
                vox_destroy_tree (tree);
            }
            goto done;
        }
        if (tree->flags & DENSE_LEAF) vox_box_copy (&(tree->bounding_box), &bb);
    }
    else
    {
        /*
          If tree is an empty leaf, allocate a regular leaf node and store
          the voxel there.
        */
        WITH_STAT (gstats.leaf_insertions++);
        dots = alloca (sizeof (vox_dot) + 16);
        dots = (void*)(((unsigned long) dots + 15) & ~(unsigned long)15);
        vox_dot_copy (dots[0], voxel);
        node = vox_make_tree (dots, 1);
        goto done;
    }

    if (tree->flags & DENSE_LEAF)
    {
        WITH_STAT (gstats.dense_insertions++);
        if (tree->dots_num < VOX_MAX_DOTS)
        {
            /*
              Downgrade to an ordinary leaf. This will result is lesser amount
              of inner nodes.
            */
            dots = alloca (sizeof (vox_dot)*VOX_MAX_DOTS + 16);
            dots = (void*)(((unsigned long) dots + 15) & ~(unsigned long)15);
            flatten_tree (tree, dots);
            vox_dot_copy (dots[tree->dots_num], voxel);
            node = vox_make_tree (dots, tree->dots_num+1);
            vox_destroy_tree (tree);
        }
        else node = insert_in_big_dense (tree, voxel);
    }
    else if (tree->flags & LEAF)
    {
        WITH_STAT (gstats.leaf_insertions++);
        // We have enough space to add a voxel
        if (tree->dots_num < VOX_MAX_DOTS)
        {
            vox_dot_copy (tree->data.dots[tree->dots_num], voxel);
            tree->dots_num++;
        }
        // If not, create a new subtree with vox_make_tree()
        else
        {
            assert (tree->dots_num == VOX_MAX_DOTS);
            dots = alloca (sizeof (vox_dot)*(VOX_MAX_DOTS+1) + 16);
            dots = (void*)(((unsigned long) dots + 15) & ~(unsigned long)15);
            memcpy (dots, tree->data.dots, sizeof(vox_dot)*tree->dots_num);
            vox_dot_copy (dots[tree->dots_num], voxel);
            node = vox_make_tree (dots, tree->dots_num+1);
            vox_destroy_tree (tree);
        }
    }
    else
    {
        /*
          For inner node we need only to increment node's voxel number and
          propagate changes further.
        */
        vox_inner_data *inner = &(tree->data.inner);
#ifdef SSE_INTRIN
        int idx = get_subspace_idx_simd (_mm_load_ps(inner->center), _mm_load_ps(voxel));
#else
        int idx = get_subspace_idx (inner->center, voxel);
#endif
        tree->dots_num++;
        tree_ptr = &(inner->children[idx]);
        goto again; // Force tail call optimization.
    }

done:
    *tree_ptr = node;
}

int vox_insert_voxel (struct vox_node **tree_ptr, vox_dot voxel)
{
    int res;

    vox_align (voxel);
    res = !(voxel_in_tree (*tree_ptr, voxel));
    if (res) vox_insert_voxel_ (tree_ptr, voxel);
    return res;
}

int vox_insert_voxel_coord (struct vox_node **tree_ptr, float x, float y, float z)
{
    vox_dot dot;
#ifdef SSE_INTRIN
    _mm_store_ps (dot, _mm_set_ps (0, z, y, x));
#else
    dot[0] = x; dot[1] = y; dot[2] = z;
#endif
    return vox_insert_voxel (tree_ptr, dot);
}

/*
  Helper function for deletion from dense stripes.
  It always deletes if dense leaf is a stripe
  (with bounding box of dimensions 1x1xN, 1xNx1 or Nx1x1).
  Return NULL otherwise.
*/
static struct vox_node*
delete_from_dense_stripe (struct vox_node *tree, const vox_dot voxel)
{
    int idx, stripe, success;
    struct vox_node *node, *child;
    vox_dot other_side;
    vox_inner_data *inner;
    struct vox_box bb;

    stripe = stripep (&(tree->bounding_box), &idx);
    if (!stripe) return NULL;

    // First special case. Delete "leftmost" voxel.
    if (vox_dot_equalp (tree->bounding_box.min, voxel))
    {
        tree->dots_num--;
        tree->bounding_box.min[idx] += vox_voxel[idx];
        return tree;
    }
    vox_sum_vector (voxel, vox_voxel, other_side);
    // First special case. Delete "rightmost" voxel.
    if (vox_dot_equalp (tree->bounding_box.max, other_side))
    {
        tree->dots_num--;
        tree->bounding_box.max[idx] -= vox_voxel[idx];
        return tree;
    }

    // Common case, create 1 inner node.
    node = node_alloc (0);
    inner = &(node->data.inner);

    vox_box_copy (&(node->bounding_box), &(tree->bounding_box));
    node->flags = 0;
    node->dots_num = tree->dots_num - 1;
    vox_dot_copy (inner->center, voxel);
    memset (inner->children, 0, sizeof (inner->children));

    success = divide_box (&(tree->bounding_box), voxel, &bb, 1<<idx);
    assert (success);
    child = vox_make_dense_leaf (&bb);
    inner->children[1<<idx] = child;

    success = divide_box (&(tree->bounding_box), voxel, &bb, 0);
    assert (success);
    child = vox_make_dense_leaf (&bb);
    inner->children[0] = child;
    assert (inner->children[0]->dots_num + inner->children[1<<idx]->dots_num ==
            tree->dots_num);
    child->dots_num--;
    child->bounding_box.min[idx] += vox_voxel[idx];
    vox_destroy_tree (tree);
    return node;
}

static struct vox_node* __attribute__((noinline))
    delete_from_big_dense (struct vox_node *tree, const vox_dot voxel)
{
    struct vox_node *node;
    vox_inner_data *inner;
    struct vox_box bb;
    int i, success;
    vox_dot other_side;

    // Maybe we are deleting from stripe.
    node = delete_from_dense_stripe (tree, voxel);
    if (node != NULL) return node;

    // No, special case 1, deletion of "leftmost" voxel.
    if (vox_dot_equalp (tree->bounding_box.min, voxel))
    {
        vox_sum_vector (voxel, vox_voxel, other_side);
        // Create 1 inner node.
        node = node_alloc (0);
        inner = &(node->data.inner);
        vox_box_copy (&(node->bounding_box), &(tree->bounding_box));
        node->flags = 0;
        node->dots_num = tree->dots_num - 1;
        vox_dot_copy (inner->center, other_side);
        memset (inner->children, 0, sizeof (inner->children));

        for (i=0; i<VOX_NS-1; i++)
        {
            success = divide_box (&(tree->bounding_box), other_side, &bb, i);
            if (!success) continue;
            inner->children[i] = vox_make_dense_leaf (&bb);
        }
        vox_destroy_tree (tree);
        return node;
    }

    // Most common case, create 2 nodes
    node = node_alloc (0);
    inner = &(node->data.inner);
    vox_box_copy (&(node->bounding_box), &(tree->bounding_box));
    node->flags = 0;
    node->dots_num = tree->dots_num - 1;
    vox_dot_copy (inner->center, voxel);
    memset (inner->children, 0, sizeof (inner->children));

    for (i=0; i<VOX_NS; i++)
    {
        success = divide_box (&(tree->bounding_box), voxel, &bb, i);
        if (!success) continue;
        inner->children[i] = vox_make_dense_leaf (&bb);
    }

    /*
      Delete voxel using a special case in this function,
      also deleting the original tree.

      May be easily TCO'ed if needed.
    */
    inner->children[0] = delete_from_big_dense (inner->children[0], voxel);
    vox_destroy_tree (tree);
    return node;
}

// It always deletes.
static void vox_delete_voxel_ (struct vox_node **tree_ptr, const vox_dot voxel)
{
    struct vox_node *tree, *node;
    int i;

again:
    tree = *tree_ptr;
    if (tree->dots_num == 1)
    {
        vox_destroy_tree (tree);
        *tree_ptr = NULL;
        return;
    }

    // XXX: update bounding box

    if (tree->flags & LEAF)
    {
        WITH_STAT (gstats.leaf_deletions++);
        for (i=0; i<tree->dots_num; i++)
            if (vox_dot_equalp (tree->data.dots[i], voxel)) break;
        assert (i < tree->dots_num);
        tree->dots_num--;
        memmove (tree->data.dots + i, tree->data.dots + i + 1,
                 sizeof (vox_dot) * (tree->dots_num - i));
    }
    else if (tree->flags & DENSE_LEAF)
    {
        WITH_STAT (gstats.dense_deletions++);
        // Downgrade to an ordinary leaf
        if (tree->dots_num <= VOX_MAX_DOTS)
        {
            // Give it a try, what if this is the case?
            node = delete_from_dense_stripe (tree, voxel);
            if (node)
            {
                *tree_ptr = node;
                return;
            }
            vox_dot *set;
            set = alloca (sizeof (vox_dot)*VOX_MAX_DOTS + 16);
            set = (void*)(((unsigned long) set + 15) & ~(unsigned long)15);
            flatten_tree (tree, set);
            /*
              XXX: Proper index may be calculated faster, if needed.
              Just find it using loop now
            */
            for (i=0; i<tree->dots_num; i++)
                if (vox_dot_equalp (set[i], voxel)) break;
            assert (i < tree->dots_num);
            memmove (set + i, set + i + 1,
                     sizeof (vox_dot) * (tree->dots_num - i - 1));
            *tree_ptr = vox_make_tree (set, tree->dots_num - 1);
            vox_destroy_tree (tree);
        }
        /*
          Divide our dense leaf into many dense leafs in 1 or 2 inner
          nodes or return a new dense leaf as a special case
        */
        else *tree_ptr = delete_from_big_dense (tree, voxel);
    }
    else
    {
        // Inner node
        vox_inner_data *inner = &(tree->data.inner);
#ifdef SSE_INTRIN
        int idx = get_subspace_idx_simd (_mm_load_ps(inner->center), _mm_load_ps(voxel));
#else
        int idx = get_subspace_idx (inner->center, voxel);
#endif
        node = inner->children[idx];
        if (node->dots_num == tree->dots_num)
        {
            /*
              This means that this child is the only child of its parent.
              So we can remove this parent and connect the child to parent's
              parent directly.
            */
            *tree_ptr = node;
            free (tree); // Save the child.
            goto again;
        }
        tree->dots_num--;
        tree_ptr = &(inner->children[idx]);
        goto again;
    }
}

int vox_delete_voxel (struct vox_node **tree_ptr, vox_dot voxel)
{
    int res;

    vox_align (voxel);
    res = voxel_in_tree (*tree_ptr, voxel);
    if (res) vox_delete_voxel_ (tree_ptr, voxel);
    return res;
}

int vox_delete_voxel_coord (struct vox_node **tree_ptr, float x, float y, float z)
{
    vox_dot dot;
#ifdef SSE_INTRIN
    _mm_store_ps (dot, _mm_set_ps (0, z, y, x));
#else
    dot[0] = x; dot[1] = y; dot[2] = z;
#endif
    return vox_delete_voxel (tree_ptr, dot);
}

void vox_dump_tree (const struct vox_node *tree)
{
    const char *leaf_str = "LEAF";
    const char *dense_str = "DENSE LEAF";
    const char *inner_str = "INNER";
    const char *desc;
    int i;

    if (VOX_FULLP (tree))
    {
        if (tree->flags & LEAF) desc = leaf_str;
        else if (tree->flags & DENSE_LEAF) desc = dense_str;
        else desc = inner_str;

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

        if (tree->flags & LEAF)
        {
            printf ("Dots:\n");
            for (i=0; i<tree->dots_num; i++)
                printf ("<%f, %f, %f>\n",
                        tree->data.dots[i][0],
                        tree->data.dots[i][1],
                        tree->data.dots[i][2]);
        }
        else if (!(tree->flags & LEAF_MASK))
        {
            printf ("Center <%f, %f, %f>\n",
                    tree->data.inner.center[0],
                    tree->data.inner.center[1],
                    tree->data.inner.center[2]);
            printf ("Children:\n");
            for (i=0; i<VOX_NS; i++) printf ("%p\n", tree->data.inner.children[i]);
        }
        printf ("------Node ends here------\n");
        if (!(tree->flags & LEAF_MASK))
        {
            for (i=0; i<VOX_NS; i++) vox_dump_tree (tree->data.inner.children[i]);
            printf ("=======Node and children end here=======\n");
        }
    }
}
