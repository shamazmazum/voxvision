#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>

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

static void* node_alloc (int leaf)
{
    /*
      FIXME: We may need to be sure if vox_dot fields of node
      structure are properly aligned in future, so use aligned_alloc()
      instead of malloc()
    */
    size_t alloc_base = offsetof (struct vox_node, data);
    size_t size = alloc_base + ((leaf) ? sizeof (vox_dot*) : sizeof (vox_inner_data));
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
    int leaf = n <= VOX_MAX_DOTS;
    struct vox_node *res  = NULL;
    WITH_STAT (recursion++);

    if (n > 0)
    {
        res = node_alloc (leaf);
        res->dots_num = n;
        calc_bounding_box (set, n, &(res->bounding_box));

        if (leaf)
        {
            WITH_STAT (gstats.leaf_nodes++);
            WITH_STAT (gstats.depth_hist[recursion]++);
            res->data.dots = set;
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

static int vox_inacc_depth_ (struct vox_node *tree, int res)
{
    if (VOX_LEAFP (tree)) return res;
    else return vox_inacc_depth_ (tree->data.inner.children[res&(VOX_NS-1)], res+1);
}

int vox_inacc_depth (struct vox_node *tree)
{
    return vox_inacc_depth_ (tree, 0);
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

/* Taken from my voxel-octrees library for common lisp:

   ;; FIXME: Logical derivations are so:
   ;;  1) We presume that each leaf in the tree contains
   ;;     the same number of elements (more precise:
   ;;     (1 + *max-dots*) / 2.
   ;;  2) We also think that number of nodes between the root
   ;;     and any leaf of tree (no matter the path we follow)
   ;;     is the same too.
   ;;
   ;;  1) and 2) are true for "idealy" balanced trees.
   ;;  The INACCURATE-BALANCENESS exploits these two assumptions
   ;;  and takes ratio of tree depth calculated both by using
   ;;  known number of voxels in the tree and by traversing
   ;;  the tree from root to leaf incrementing the depth by 1.
   ;;
   ;;  It is designed to return 1 if TREE conformes both
   ;;  1) and 2) statements.
   ;;
   ;;  So if the TREE is balanced the function returns 1.
   ;;  In other words, if INACCURATE-BALANCENESS does not
   ;;  return 1 if the tree is unbalanced.
   
   ;; The following function called "inaccurate" because
   ;; it uses two assumptions listed above and does not
   ;; establish (two-way) equality between the fact that
   ;; tree is balanced and equality of its result to 1
*/
   
float vox_inacc_balanceness (struct vox_node *tree)
{
    float expected_depth = ceilf (log (2.0 * vox_voxels_in_tree (tree) / (1 + VOX_MAX_DOTS)) / log (VOX_NS));
    return vox_inacc_depth (tree) / expected_depth;
}

void vox_bounding_box (const struct vox_node* tree, struct vox_box *box)
{
    vox_dot_copy (box->min, tree->bounding_box.min);
    vox_dot_copy (box->max, tree->bounding_box.max);
}
