#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <assert.h>
#include <stdio.h>

#include "tree.h"
#include "geom.h"

vox_dot vox_voxel = {1.0, 1.0, 1.0};

#ifdef SSE_INTRIN
static void calc_avg (const vox_dot set[], size_t n, vox_dot res)
{
    size_t i;
    __v4sf len = _mm_set_ps1 (n);
    __v4sf resv = _mm_set_ps1 (0.0);

    for (i=0; i<n; i++) resv += _mm_load_ps (set[i]);
    resv /= len;

    _mm_store_ps (res, resv);
}
#else /* SSE_INTRIN */
static void calc_avg (const vox_dot set[], size_t n, vox_dot res)
{
    size_t i;
    bzero (res, sizeof(vox_dot));

    for (i=0; i<n; i++) sum_vector (set[i], res, res);
    for (i=0; i<VOX_N; i++) res[i] /= n;
}
#endif /* SSE_INTRIN */

#ifdef SSE_INTRIN
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
#else /* SSE_INTRIN */
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
    sum_vector (box->max, vox_voxel, box->max);
}
#endif /* SSE_INTRIN */

static void vox_align_ceil (vox_dot dot)
{
    int i;
    float tmp;

    for (i=0; i<VOX_N; i++)
    {
        tmp = ceilf (dot[i] / vox_voxel[i]);
        dot[i] = vox_voxel[i] * tmp;
    }
}

static void vox_align_floor (vox_dot dot)
{
    int i;
    float tmp;

    for (i=0; i<VOX_N; i++)
    {
        tmp = floorf (dot[i] / vox_voxel[i]);
        dot[i] = vox_voxel[i] * tmp;
    }
}

/*
  We cannot use this version in the entire library, as
   data can be accessed almost randomly when raycasting
   is performed. But for sorting purposed it is great for
   performance. Note, that clang inlines this function
*/
#ifdef SSE_INTRIN
static int get_subspace_idx_simd (const vox_dot center, const vox_dot dot)
{
    __v4sf sub = _mm_load_ps (dot);
    sub -= _mm_load_ps (center);
    return (_mm_movemask_ps (sub) & 0x07);
}
#endif /* SSE_INTRIN */

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
#ifdef SSE_INTRIN
        if (get_subspace_idx_simd (center, set[i]) == subspace)
#else
        if (get_subspace_idx (center, set[i]) == subspace)
#endif
        {
            vox_dot_copy (tmp, set[counter]);
            vox_dot_copy (set[counter], set[i]);
            vox_dot_copy (set[i], tmp);
            counter++;
        }
    }
    
    return counter;
}

static void* node_alloc (int flavor)
{
    /*
      FIXME: We may need to be sure if vox_dot fields of node
      structure are properly aligned in future, so use aligned_alloc()
      instead of malloc()
    */
    size_t size = offsetof (struct vox_node, data);
    if (flavor & LEAF) size += sizeof (vox_dot*);
    else if (!(flavor & DENSE_LEAF)) size += sizeof (vox_inner_data);
    return aligned_alloc (16, size);
}

WITH_STAT (static int recursion = -1;)

static struct vox_node* make_dense_leaf (const struct vox_box *box)
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
    struct vox_node *res  = NULL;
    WITH_STAT (recursion++);

    if (n > 0)
    {
        struct vox_box box;
        calc_bounding_box (set, n, &box);
        int densep = (dense_set_p (&box, n)) ? DENSE_LEAF : 0;
        int leafp = (n <= VOX_MAX_DOTS) ? LEAF : 0;
        res = node_alloc (leafp | densep);
        res->dots_num = n;
        res->flags = 0;
        vox_box_copy (&(res->bounding_box), &box);
        if (densep)
        {
            res->flags |= DENSE_LEAF;
            WITH_STAT (gstats.dense_leafs++);
            WITH_STAT (gstats.dense_dots+=n);
        }
        else if (leafp)
        {
            WITH_STAT (gstats.leaf_nodes++);
            WITH_STAT (gstats.depth_hist[recursion]++);
            res->data.dots = aligned_alloc (16, VOX_MAX_DOTS*sizeof(vox_dot));
            memcpy (res->data.dots, set, n*sizeof(vox_dot));
            res->flags |= LEAF;
            WITH_STAT (float ratio = fill_ratio (&(res->bounding_box), n));
            WITH_STAT (update_fill_ratio_hist (ratio));
            WITH_STAT (gstats.empty_volume += get_empty_volume (ratio, n));
        }
        else
        {
            int idx;
            vox_inner_data *inner = &(res->data.inner);
            size_t new_offset, offset = 0;

            calc_avg (set, n, inner->center);
            /*
              Align the center of division, so any voxel belongs to only one subspace
              entirely. Faces of voxels may be the exception though
            */
            vox_align_ceil (inner->center);

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
    WITH_STAT (else gstats.empty_nodes++);
    WITH_STAT (recursion--);
    return res;
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

#ifdef SSE_INTRIN
static void update_bounding_box (struct vox_box *box, const vox_dot dot)
{
    __v4sf d = _mm_load_ps (dot);
    __v4sf d_max = d + _mm_load_ps (vox_voxel);

    __v4sf box_min = _mm_min_ps (d, _mm_load_ps (box->min));
    __v4sf box_max = _mm_max_ps (d_max, _mm_load_ps (box->max));

    _mm_store_ps (box->min, box_min);
    _mm_store_ps (box->max, box_max);
}
#else
static void update_bounding_box (struct vox_box *box, const vox_dot dot)
{
    vox_dot dot_max;
    int i;
    sum_vector (dot, vox_voxel, dot_max);

    for (i=0; i<VOX_N; i++)
    {
        box->min[i] = (dot[i] < box->min[i]) ? dot[i] : box->min[i];
        box->max[i] = (dot_max[i] > box->max[i]) ? dot_max[i] : box->max[i];
    }
}
#endif

static int vox_insert_voxel_ (struct vox_node **tree_ptr, const vox_dot voxel);
static struct vox_node* __attribute__((noinline))
    insert_in_big_dense (struct vox_node *tree, const vox_dot voxel)
{
    /*
      Put a dense leaf and this voxel in a new inner node.
      Benchmarks show that this is the slowest part of this function
      (especially when compiled with SSE_INTRIN).
    */
    vox_dot inner_dot;
    int idx1, idx2;
    int i;
    struct vox_node *node;
    vox_inner_data *inner;

    node = node_alloc (0);
    inner = &(node->data.inner);
    bzero (inner, sizeof (vox_inner_data));
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
    _mm_store_ps (inner_dot, inner_vector);
    idx1 = get_subspace_idx_simd (inner->center, inner_dot);
    idx2 = get_subspace_idx_simd (inner->center, voxel);
#else
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

static int vox_insert_voxel_ (struct vox_node **tree_ptr, const vox_dot voxel)
{
    struct vox_node *tree = *tree_ptr;
    int res = 0;
    vox_dot *dots;
    struct vox_node *node = tree;

    // If tree is an empty leaf, allocate a regular leaf node and store the voxel there
    if (!(VOX_FULLP (tree)))
    {
        WITH_STAT (gstats.leaf_insertions++);
        dots = alloca (sizeof (vox_dot) + 16);
        dots = (void*)(((unsigned long) dots + 15) & ~(unsigned long)15);
        vox_dot_copy (dots[0], voxel);
        node = vox_make_tree (dots, 1);
        res = 1;
    }
    else if (tree->flags & DENSE_LEAF)
    {
        if (!(voxel_in_box (&(tree->bounding_box), voxel)))
        {
            WITH_STAT (gstats.dense_insertions++);
            res = 1;
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
            }
            else node = insert_in_big_dense (tree, voxel);
        }
    }
    else if (tree->flags & LEAF)
    {
        int i;
        // Check if the voxel is already in the tree
        for (i=0; i<tree->dots_num; i++)
        {
            if (vox_dot_equalp (tree->data.dots[i], voxel)) return 0;
        }
        WITH_STAT (gstats.leaf_insertions++);
        res = 1;
        // We have enough space to add a voxel
        if (tree->dots_num < VOX_MAX_DOTS)
        {
            update_bounding_box (&(tree->bounding_box), voxel);
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
        // For inner node we need only to upgrade bounding box
        vox_inner_data *inner = &(tree->data.inner);
        int idx = get_subspace_idx (inner->center, voxel);
        update_bounding_box (&(tree->bounding_box), voxel);
        if (vox_insert_voxel_ (&(inner->children[idx]), voxel))
        {
            res = 1;
            tree->dots_num++;
        }
    }
    *tree_ptr = node;
    return res;
}

int vox_insert_voxel (struct vox_node **tree_ptr, vox_dot voxel)
{
    vox_align_floor (voxel);
    return vox_insert_voxel_ (tree_ptr, voxel);
}

/*
  Helper function for dense node special case.
  It always deletes.
*/
static struct vox_node* __attribute__((noinline))
    delete_from_big_dense (struct vox_node *tree, const vox_dot voxel)
{
    struct vox_node *node1, *node2;
    size_t n1 = 0, n2 = 0;
    vox_inner_data *inner;
    vox_dot other_end;
    struct vox_box bb;
    int full, i;

    sum_vector (voxel, vox_voxel, other_end);
    node1 = node_alloc (0);
    /*
      XXX: Just copy the bounding box. It can be smaller, actually.
      But it is hard to determine.
    */
    vox_box_copy (&(node1->bounding_box), &(tree->bounding_box));
    node1->flags = 0;
    inner = &(node1->data.inner);
    vox_dot_copy (inner->center, voxel);
    bzero (inner->children, sizeof (inner->children));
    full = 0;

    for (i=1; i<VOX_NS; i++)
    {
        int success = divide_box (&(node1->bounding_box), voxel, &bb, i);
        if (success == 0) continue;
        full = 1;
        inner->children[i] = make_dense_leaf (&bb);
        n1 += inner->children[i]->dots_num;
    }

    if (full)
    {
        node2 = node_alloc (0);
        inner->children[0] = node2;
    }
    else node2 = node1;

    // Sorry for boilerplate code. Must be reworked
    vox_dot_copy (node2->bounding_box.min, voxel);
    vox_dot_copy (node2->bounding_box.max, tree->bounding_box.max);
    node2->flags = 0;
    inner = &(node2->data.inner);
    vox_dot_copy (inner->center, other_end);
    bzero (inner->children, sizeof (inner->children));
    full = 0;

    for (i=0; i<VOX_NS-1; i++)
    {
        int success = divide_box (&(node2->bounding_box), other_end, &bb, i);
        if (success == 0) continue;
        full = 1;
        inner->children[i] = make_dense_leaf (&bb);
        n2 += inner->children[i]->dots_num;
    }
    node2->dots_num = n2;
    node1->dots_num = n1+n2;

    if (!full)
    {
        assert (node1 != node2);
        vox_destroy_tree (node2);
        node1->data.inner.children[0] = NULL;
    }

    assert (node1->dots_num == tree->dots_num - 1);

    return node1;
}

static int vox_delete_voxel_ (struct vox_node **tree_ptr, const vox_dot voxel)
{
    int res = 0;
    struct vox_node *tree = *tree_ptr;
    struct vox_node *node = tree;
    int i;

    if (VOX_FULLP (tree) &&
        voxel_in_box (&(tree->bounding_box), voxel))
    {
        // XXX: update bounding box
        if (tree->flags & LEAF)
        {
            for (i=0; i<tree->dots_num; i++)
                if (vox_dot_equalp (tree->data.dots[i], voxel)) break;
            if (i < tree->dots_num)
            {
                WITH_STAT (gstats.leaf_deletions++);
                res = 1;
                memmove (tree->data.dots + i, tree->data.dots + i + 1,
                         sizeof (vox_dot) * (tree->dots_num - i - 1));
                tree->dots_num--;
                if (tree->dots_num == 0)
                {
                    vox_destroy_tree (tree);
                    node = NULL;
                }
            }
        }
        else if (tree->flags & DENSE_LEAF)
        {
            WITH_STAT (gstats.dense_deletions++);
            res = 1;
            // Downgrade to an ordinary leaf or even destroy the node
            if (tree->dots_num <= VOX_MAX_DOTS)
            {
                if (tree->dots_num == 1)
                {
                    vox_destroy_tree (tree);
                    node = NULL;
                }
                else
                {
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
                    node = vox_make_tree (set, tree->dots_num - 1);
                    vox_destroy_tree (tree);
                }
            }
            else
            {
                /*
                  Divide our dense leaf into many dense leafs in 1 or 2 inner
                  nodes.
                */
                node = delete_from_big_dense (tree, voxel);
                vox_destroy_tree (tree);
            }
        }
        else
        {
            // Inner node
            vox_inner_data *inner = &(tree->data.inner);
            int idx = get_subspace_idx (inner->center, voxel);
            res = vox_delete_voxel_ (&(inner->children[idx]), voxel);
            if (res)
            {
                tree->dots_num--;
                if (tree->dots_num == 0)
                {
                    vox_destroy_tree (tree);
                    node = NULL;
                }
            }
        }
    }

    *tree_ptr = node;
    return res;
}

int vox_delete_voxel (struct vox_node **tree_ptr, vox_dot voxel)
{
    vox_align_floor (voxel);
    return vox_delete_voxel_ (tree_ptr, voxel);
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
            for (i=0; i<VOX_NS; i++) vox_dump_tree (tree->data.inner.children[i]);
        printf ("=======Node and children end here=======\n");
    }
}
