#include <string.h>
#include <math.h>
#if 0
#include <gc.h>
#else
#include <stdlib.h>
#endif

#include "tree.h"
#include "geom.h"

//#include <stdlib.h>

vox_dot vox_voxel = {1.0, 1.0, 1.0};

/**
   \brief Calc average dot for the set.
   
   \param n a number of dots in the set
   \param res an array where the result is stored
   \return Third passed argument, whichs contains the average
**/
static void calc_avg (vox_dot set[], vox_uint n, vox_dot res)
{
    vox_uint i;
    float lenmul = 1.0/n;
    memset (res, 0, sizeof(vox_dot));

    for (i=0; i<n; i++) sum_vector (set[i], res, res);
    for (i=0; i<VOX_N; i++) res[i] *= lenmul;
}

/**
   \brief Calculate the minimal cuboid hull for set of voxels.
   
   \param set the set of voxels
   \param n a number of voxels in the set
   \param min an array where the minimal coordinate is stored
   \param max an array where the maximal coordinate is stored
**/
static void calc_bounding_box (vox_dot set[], vox_uint n, vox_dot min, vox_dot max)
{
    vox_uint i,j;
    
    memcpy (min, set[0], sizeof(vox_dot));
    memcpy (max, set[0], sizeof(vox_dot));

    for (i=0; i<n; i++)
    {
        for (j=0; j<VOX_N; j++)
        {
            if (set[i][j] < min[j]) min[j] = set[i][j];
            else if (set[i][j] > max[j]) max[j] = set[i][j];
        }
    }
    sum_vector (max, vox_voxel, max);
}

void vox_align (vox_dot dot)
{
    vox_uint i;
    float tmp;

    for (i=0; i<VOX_N; i++)
    {
        tmp = ceilf (dot[i] / vox_voxel[i]);
        dot[i] = vox_voxel[i] * tmp;
    }
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
static vox_uint filter_set (vox_dot set[], vox_uint n, vox_uint offset, vox_uint subspace, const vox_dot center)
{
    vox_uint i;
    vox_dot tmp;
    vox_uint counter = offset;

    for (i=offset; i<n; i++)
    {
        if (get_subspace_idx (center, set[i]) == subspace)
        {
            memcpy (tmp, set[counter], sizeof (vox_dot));
            memcpy (set[counter], set[i], sizeof (vox_dot));
            memcpy (set[i], tmp, sizeof (vox_dot));
            counter++;
        }
    }
    
    return counter;
}

// Self-explanatory. No, really.
// Being short: if number of voxels in a set is less or equal
// to maximum number allowed, create a leaf and store voxels there.
// Otherwise split the set into 2^N parts and proceed with each of subsets
// recursively.
struct vox_node* vox_make_tree (vox_dot set[], vox_uint n)
{
#if 0
    struct vox_node *res  = GC_MALLOC(sizeof(struct vox_node));
#else
    struct vox_node *res  = malloc (sizeof(struct vox_node));
#endif
    res->flags = 0;
    if (n > 0)
    {
        res->flags |= 1<<VOX_FULL;
        calc_bounding_box (set, n, res->bb_min, res->bb_max);
    }
    
    if (n <= VOX_MAX_DOTS)
    {
        res->flags |= 1<<VOX_LEAF;
        vox_leaf_data *leaf = &(res->data.leaf);
        leaf->dots_num = n;
        leaf->dots = set;
    }
    else
    {
        vox_uint idx;
        vox_inner_data *inner = &(res->data.inner);
        vox_uint new_offset, offset;
        offset = 0;
        
        calc_avg (set, n, inner->center);
        vox_align (inner->center);

        for (idx=0; idx<VOX_NS; idx++)
        {
            new_offset = filter_set (set, n, offset, idx, inner->center);
            inner->children[idx] = vox_make_tree (set+offset, new_offset-offset);
            offset = new_offset;
        }
    }
    return res;
}

vox_uint vox_voxels_in_tree (struct vox_node *tree)
{
    vox_uint res, i;
    if (VOX_LEAFP (tree)) res = tree->data.leaf.dots_num;
    else
    {
        res = 0;
        for (i=0; i<VOX_NS; i++) res += vox_voxels_in_tree (tree->data.inner.children[i]);
    }
    return res;
}

vox_uint vox_inacc_depth (struct vox_node *tree, vox_uint res)
{
    if (VOX_LEAFP (tree)) return res;
    else return vox_inacc_depth (tree->data.inner.children[res&(VOX_NS-1)], res+1);
}

void vox_destory_tree (struct vox_node *tree)
{
    if (!(VOX_LEAFP (tree)))
    {
        vox_uint i;
        for (i=0; i<VOX_NS; i++) vox_destory_tree (tree->data.inner.children[i]);
    }
#if 0
    GC_FREE (tree);
#else
    free (tree);
#endif
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
    return vox_inacc_depth (tree, 0) / expected_depth;
}
