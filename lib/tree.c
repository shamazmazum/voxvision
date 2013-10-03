#include <string.h>
#include <stdint.h>
#include <math.h>
#include <gc.h>

#include "tree.h"

//#include <stdlib.h>

float voxel[3] = {1.0, 1.0, 1.0};

float *sum_vector (const float *a, const float *b, float *res)
{
    int i;
    for (i=0; i<N; i++) res[i] = a[i] + b[i];
    return res;
}

/**
   \brief Calc average dot for the set.
   
   \param n a number of dots in the set
   \param res an array where the result is stored
   \return Third passed argument, whichs contains the average
**/
static float* calc_avg (float set[][N], unsigned int n, float res[N])
{
    int i;
    float lenmul = 1.0/n;
    memset (res, 0, sizeof(float)*N);

    for (i=0; i<n; i++) sum_vector (set[i], res, res);
    for (i=0; i<N; i++) res[i] *= lenmul;
    return res;
}

/**
   \brief Calculate the minimal cuboid hull for set of voxels.
   
   \param set the set of voxels
   \param n a number of voxels in the set
   \param min an array where the minimal coordinate is stored
   \param max an array where the maximal coordinate is stored
**/
static void calc_bounding_box (float set[][N], unsigned int n, float min[N], float max[N])
{
    int i,j;
    
    memcpy (min, set[0], sizeof(float)*N);
    memcpy (max, set[0], sizeof(float)*N);

    for (i=0; i<n; i++)
    {
        for (j=0; j<N; j++)
        {
            if (set[i][j] < min[j]) min[j] = set[i][j];
            else if (set[i][j] > max[j]) max[j] = set[i][j];
        }
    }
    sum_vector (max, voxel, max);
}

uint8_t get_subspace_idx (const float *dot1, const float *dot2)
{
    uint8_t res = 0;
    int i;

    for (i=0; i<N; i++) res |= ((dot1[i] > dot2[i]) ? 1 : 0) << i;
    return res;
}

/**
   \brief Align the dot on voxel if needed.
   \return Passed argument
**/
static float* align_on_voxel (float *dot)
{
    int i;
    float tmp;

    for (i=0; i<N; i++)
    {
        tmp = ceilf (dot[i] / voxel[i]);
        dot[i] = voxel[i] * tmp;
    }
    return dot;
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
static unsigned int filter_set (float set[][N], unsigned int n, unsigned int offset, uint8_t subspace, const float center[N])
{
    int i;
    float tmp[N];
    unsigned int counter = offset;

    for (i=offset; i<n; i++)
    {
        if (get_subspace_idx (center, set[i]) == subspace)
        {
            memcpy (tmp, set[counter], sizeof (float)*N);
            memcpy (set[counter], set[i], sizeof (float)*N);
            memcpy (set[i], tmp, sizeof (float)*N);
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
struct node* make_tree (float set[][N], unsigned int n)
{
    struct node *res  = GC_MALLOC(sizeof(struct node));
    res->flags = 0;
    if (n > 0)
    {
        res->flags |= 1<<FULL;
        calc_bounding_box (set, n, res->bb_min, res->bb_max);
    }
    
    if (n <= MAX_DOTS)
    {
        res->flags |= 1<<LEAF;
        leaf_data *leaf = &(res->data.leaf);
        leaf->dots_num = n;
        leaf->dots = set;
    }
    else
    {
        int idx;
        inner_data *inner = &(res->data.inner);
        unsigned int new_offset, offset;
        offset = 0;
        
        calc_avg (set, n, inner->center);
        align_on_voxel (inner->center);

        for (idx=0; idx<NS; idx++)
        {
            new_offset = filter_set (set, n, offset, idx, inner->center);
            inner->children[idx] = make_tree (set+offset, new_offset-offset);
            offset = new_offset;
        }
    }
    return res;
}

unsigned int voxels_in_tree (struct node *tree)
{
    unsigned int res, i;
    if (LEAFP (tree)) res = tree->data.leaf.dots_num;
    else
    {
        res = 0;
        for (i=0; i<NS; i++) res += voxels_in_tree (tree->data.inner.children[i]);
    }
    return res;
}

unsigned int inacc_depth (struct node *tree, unsigned int res)
{
    if (LEAFP (tree)) return res;
    else return inacc_depth (tree->data.inner.children[res&(NS-1)], res+1);
}

/*void destroy_tree (struct node *tree)
{
    if (!(LEAFP (tree)))
    {
        int i;
        for (i=0; i<NS; i++) destroy_tree (tree->children[i]);
    }
    free (tree);
}*/

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
   
float inacc_balanceness (struct node *tree)
{
    float expected_depth = ceilf (log (2.0 * voxels_in_tree (tree) / (1 + MAX_DOTS)) / log (NS));
    return inacc_depth (tree, 0) / expected_depth;
}
