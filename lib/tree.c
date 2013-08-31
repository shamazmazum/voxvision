#include <string.h>
#include <stdint.h>
#include <math.h>

#include "tree.h"

#include <stdlib.h>

float voxel[3] = {1.0, 1.0, 1.0};

void sum_vector (const float *a, const float *b, float *res)
{
    int i;
    for (i=0; i<N; i++) res[i] = a[i] + b[i];
}

void calc_avg (float set[][N], int n, float res[N])
{
    int i;
    float lenmul = 1.0/n;
    memset (res, 0, sizeof(float)*N);

    for (i=0; i<n; i++) sum_vector (set[i], res, res);
    for (i=0; i<N; i++) res[i] *= lenmul;
}

void calc_bounding_box (float set[][N], int n, float min[N], float max[N])
{
    int i,j;
    
    memcpy (min, set[1], sizeof(float)*N);
    memcpy (max, set[1], sizeof(float)*N);

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

void align_on_voxel (float *dot)
{
    int i;
    float tmp;

    for (i=0; i<N; i++)
    {
        tmp = ceilf (dot[i] / voxel[i]);
        dot[i] = voxel[i] * tmp;
    }
}

struct node* new_node ()
{
    struct node *res = malloc (sizeof (struct node));
    res->children = NULL;
    res->dots_num = 0;
    return res;
}

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

struct node* make_tree (float set[][N], int n)
{
    struct node *res  = new_node();
    if (n > 0) calc_bounding_box (set, n, res->bb_min, res->bb_max);
    if (n < MAX_DOTS)
    {
        res->dots_num = n;
        memcpy (res->dots, set, n*sizeof(float)*N);
    }
    else
    {
        int idx;
        res->children = malloc (sizeof(struct node*) * NS);
        calc_avg (set, n, res->dots[0]);
        align_on_voxel (res->dots[0]);
        float (*subset)[N] = malloc (sizeof(float)*n*N);
        for (idx=0; idx<NS; idx++)
        {
            int sub_n = n;
            filter_set (set, subset, &sub_n, idx, res->dots[0]);
            res->children[idx] = make_tree (subset, sub_n);
        }
        free (subset);
    }
    return res;
}

int voxels_in_tree (struct node *tree)
{
    int res, i;
    if (LEAFP (tree)) res = tree->dots_num;
    else
    {
        res = 0;
        for (i=0; i<NS; i++) res += voxels_in_tree (tree->children[i]);
    }
    return res;
}

int inacc_depth (struct node *tree, int res)
{
    if (LEAFP (tree)) return res;
    else return inacc_depth (tree->children[res&(NS-1)], res+1);
}

void destroy_tree (struct node *tree)
{
    if (!(LEAFP (tree)))
    {
        int i;
        for (i=0; i<NS; i++) destroy_tree (tree->children[i]);
        free (tree->children);
    }
    free (tree);
}

float inacc_balanceness (struct node *tree)
{
    float expected_depth = ceilf (log (2.0 * voxels_in_tree (tree) / (1 + MAX_DOTS)) / log (NS));
    return inacc_depth (tree, 0) / expected_depth;
}
