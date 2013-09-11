/**
   @file tree.h
   @brief Things related to tree construction

   Functions for tree construction/deletion/statistical info are defined here
**/

#include <string.h>
#include <stdint.h>
#include <math.h>
#include <gc.h>

#include "tree.h"

//#include <stdlib.h>

float voxel[3] = {1.0, 1.0, 1.0};

/**
   \brief Take sum of two vectors.
   
   \param res an array where the result is stored
   \return Third passed argument, whichs contains the sum
**/
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
float* calc_avg (float set[][N], int n, float res[N])
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
void calc_bounding_box (float set[][N], int n, float min[N], float max[N])
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

/**
   \brief Calculate a subspace index for the dot.
   
   For N-dimentional space we have 2^N ways to place the dot
   around the center of subdivision. Find which way is the case.
   
   \param dot1 the center of subdivision
   \param dot2 the dot we must calculate index for
   \return The subspace index in the range [0,2^N-1]
**/
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
float* align_on_voxel (float *dot)
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
   \brief Collect only those dots which are placed in desired subspace.
   \param in a set to be filtered
   \param out the result
   \param n points to size of the initial set, becomes a size of the result an exit
   \param subspace a desired subspace
   \param center the center of subdivision
**/
void filter_set (float in[][N], float out[][N], int *n, uint8_t subspace, const float center[N])
{
    int i;
    int counter = 0;
    for (i=0; i<*n; i++)
    {
        if (get_subspace_idx (center, in[i]) == subspace)
        {
            memcpy (out[counter], in[i], sizeof(float)*N);
            counter++;
        }
    }
    *n = counter;
}

// Self-explanatory. No, really.
// Being short: if number of voxels in a set is less or equal
// to maximum number allowed, create a leaf and store voxels there.
// Otherwise split the set into 2^N parts and proceed with each of subsets
// recursively.

/**
   \brief Turn a set of voxels into a tree.
   \return a root node of the newly created tree
**/
struct node* make_tree (float set[][N], int n)
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
        memcpy (leaf->dots, set, n*sizeof(float)*N);
    }
    else
    {
        int idx;
        inner_data *inner = &(res->data.inner);
        calc_avg (set, n, inner->center);
        align_on_voxel (inner->center);
        float (*subset)[N] = GC_MALLOC_ATOMIC (sizeof(float)*n*N);
        for (idx=0; idx<NS; idx++)
        {
            int sub_n = n;
            filter_set (set, subset, &sub_n, idx, inner->center);
            inner->children[idx] = make_tree (subset, sub_n);
        }
    }
    return res;
}

/**
   \brief Return number of voxels in the tree.
**/
int voxels_in_tree (struct node *tree)
{
    int res, i;
    if (LEAFP (tree)) res = tree->data.leaf.dots_num;
    else
    {
        res = 0;
        for (i=0; i<NS; i++) res += voxels_in_tree (tree->data.inner.children[i]);
    }
    return res;
}

/**
   \brief Calculate a depth of the tree.
   
   This function is called inaccurate because
   it uses a predefined path from root to leaf,
   treating all other paths to any leaf having
   the same length

   \param tree the tree
   \param res an initial value of depth
   \return res + actual depth
**/
int inacc_depth (struct node *tree, int res)
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
