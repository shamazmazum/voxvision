#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

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

static void vox_align (vox_dot dot)
{
    int i;
    float tmp;

    for (i=0; i<VOX_N; i++)
    {
        tmp = ceilf (dot[i] / vox_voxel[i]);
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
        vox_dot_copy (res->bounding_box.min, box.min);
        vox_dot_copy (res->bounding_box.max, box.max);
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
            res->data.dots = set;
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
            vox_align (inner->center);

            for (idx=0; idx<VOX_NS; idx++)
            {
                /*
                  XXX: Current algorithm always divides voxels by two or more
                  subspaces. Is it true in general?
                */
                new_offset = sort_set (set, n, offset, idx, inner->center);
                inner->children[idx] = vox_make_tree (set+offset, new_offset-offset);
                offset = new_offset;
            }
        }
    }
    WITH_STAT (else gstats.empty_nodes++);
    WITH_STAT (recursion--);
    return res;
}

size_t vox_voxels_in_tree (struct vox_node *tree)
{
    return tree->dots_num;
}

void vox_destroy_tree (struct vox_node *tree)
{
    if (!(VOX_LEAFP (tree)))
    {
        int i;
        for (i=0; i<VOX_NS; i++) vox_destroy_tree (tree->data.inner.children[i]);
    }
    free (tree);
}

void vox_bounding_box (const struct vox_node* tree, struct vox_box *box)
{
    vox_dot_copy (box->min, tree->bounding_box.min);
    vox_dot_copy (box->max, tree->bounding_box.max);
}

/*
  Calculate a number of voxels in underlying array for which the tree
  has references in leaf nodes.
*/
static size_t underlying_voxels (const struct vox_node *tree)
{
    size_t n;
    int i;

    // Dense leaf has only bounding box which is like one big voxel
    if (VOX_DENSE_LEAFP (tree)) n = 0;
    else if (VOX_LEAFP (tree)) n = tree->dots_num;
    else
    {
        n = 0;
        for (i=0; i<VOX_NS; i++) n += underlying_voxels (tree->data.inner.children[i]);
    }
    return n;
}

static vox_dot* do_recopy (struct vox_node *tree, vox_dot *space)
{
    vox_dot *shifted_space;
    int i;

    if (VOX_DENSE_LEAFP (tree)) shifted_space = space;
    else if (VOX_LEAFP (tree))
    {
        memcpy (space, tree->data.dots, sizeof(vox_dot)*tree->dots_num);
        tree->data.dots = space;
        shifted_space = space + tree->dots_num;
    }
    else
    {
        for (i=0; i<VOX_NS; i++)
        {
            shifted_space = do_recopy (tree->data.inner.children[i], space);
            space = shifted_space;
        }
    }
    return shifted_space;
}

/*
  Do tree recopying. This operation creates a new underlying array, possibly
  smaller in size in which unused data is not present and used data is placed
  without gaps. Unused data is those voxels which are covered by dense leafs.
*/
vox_dot* vox_recopy_tree (struct vox_node *tree)
{
    size_t len = underlying_voxels (tree);
    vox_dot *array = malloc (sizeof (vox_dot) * len);
    do_recopy (tree, array);
    return array;
}
