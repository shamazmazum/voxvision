#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "geom.h"
#include "search.h"

void gen_subspaces (struct tagged_coord subspaces[], int n)
{
    int i;

    for (i=1; i<n; i++)  subspaces[i].tag = (1 << subspaces[i].tag) ^ subspaces[i-1].tag;
}

int compare_tagged (float *origin, struct tagged_coord *c1, struct tagged_coord *c2)
{
    float *dot1 = c1->coord;
    float *dot2 = c2->coord;

    return calc_abs_metric (origin, dot1) - calc_abs_metric (origin, dot2);
}

int ray_tree_intersection (struct node *tree, const float *origin, const float *dir, float *res, int depth, int lod)
{
    float inter[N];
    float tmp[N];
    int interp, i;

    if ((tree->dots_num == 0) && (LEAFP (tree))) return 0;
    if (!(hit_box (tree->bb_min, tree->bb_max, origin, dir, inter))) return 0;
    if (depth == lod)
    {
        memcpy (res, inter, sizeof (float)*N);
        return 1;
    }
    
    if (LEAFP(tree))
    {
        int count = 0;
        // Can we do it on stack??
        float intersect[MAX_DOTS][N];
        for (i=0; i<tree->dots_num; i++)
        {
            sum_vector (tree->dots[i], voxel, tmp);
            interp = hit_box (tree->dots[i], tmp, origin, dir, inter);
            if (interp)
            {
                memcpy (intersect[count], inter, sizeof(float)*N);
                count++;
            }
        }
        if (count)
        {
            memcpy (res, closest_in_set (intersect, count, origin, calc_abs_metric), sizeof(float)*N);
            return 1;
        }
        else return 0;
    }
    // ELSE

    struct tagged_coord plane_inter[N+1];
    int plane_counter = 1;
    plane_inter[0].tag = get_subspace_idx (tree->dots[0], inter);
    memcpy (plane_inter[0].coord, inter, sizeof(float)*N);
    for (i=0; i<N; i++)
    {
        interp = hit_plane (origin, dir, tree->dots[0], i, plane_inter[plane_counter].coord);
        plane_inter[plane_counter].tag = i;
        if (interp && (dot_betweenp (tree->bb_min, tree->bb_max, plane_inter[plane_counter].coord))) plane_counter++;
    }

    qsort_r (plane_inter+1, plane_counter-1, sizeof(struct tagged_coord), origin, compare_tagged);
    gen_subspaces (plane_inter, plane_counter);

    for (i=0; i<plane_counter; i++)
    {
        interp = ray_tree_intersection (tree->children[plane_inter[i].tag], plane_inter[i].coord, dir, res, depth+1, lod);
        if (interp) return 1;
    }
    
    return 0;
}
