#include <math.h>
#include "geom.h"

void sum_vector (const vox_dot a, const vox_dot b, vox_dot res)
{
    vox_uint i;
    for (i=0; i<VOX_N; i++) res[i] = a[i] + b[i];
}

vox_uint get_subspace_idx (const vox_dot dot1, const vox_dot dot2)
{
    vox_uint res, i;
    res = 0;

    for (i=0; i<VOX_N; i++) res |= ((dot1[i] > dot2[i]) ? 1 : 0) << i;
    return res;
}

float calc_abs_metric (const vox_dot dot1, const vox_dot dot2)
{
    vox_uint i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += fabsf (dot1[i] - dot2[i]);
    return res;
}

float calc_sqr_metric (const vox_dot dot1, const vox_dot dot2)
{
    vox_uint i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += powf (dot1[i] - dot2[i], 2.0);
    return res;
}

static int fit_into_box (const vox_dot min, const vox_dot max, const vox_dot dot, vox_dot res)
{
    vox_uint i;
    int the_same = 1;
    for (i=0; i<VOX_N; i++)
    {
        if (dot[i] < min[i])
        {
            res[i] = min[i];
            the_same = 0;
        }
        else if (dot[i] > max[i])
        {
            res[i] = max[i];
            the_same = 0;
        }
        else res[i] = dot[i];
    }
    return the_same;
}

// Most of the following code is taken from C Graphics Gems
// See C Graphics Gems code for explanation
int hit_box (const vox_dot min, const vox_dot max, const vox_dot origin, const vox_dot dir, vox_dot res)
{
    vox_dot candidate_plane;
    float max_dist, tdist;
    int i, plane_num;
    int insidep = fit_into_box (min, max, origin, candidate_plane);
    if (insidep)
    {
        vox_dot_copy (res, origin);
        return 1;
    }

    plane_num = 0;
    max_dist = -1;
    for (i=0; i<VOX_N; i++)
    {
        if ((candidate_plane[i] == origin[i]) || dir[i] == 0.0) tdist = -1.0;
        else
        {
            tdist = (candidate_plane[i] - origin[i])/dir[i];
            if (tdist > max_dist)
            {
                plane_num = i;
                max_dist = tdist;
            }
        }
    }
    if (max_dist < 0) return 0;

    for (i=0; i<VOX_N; i++)
    {
        if (i==plane_num) res[i] = candidate_plane[i];
        else
        {
            res[i] = origin[i] + max_dist*dir[i];
            if ((res[i] < min[i]) || (res[i] > max[i])) return 0;
        }
    }
    
    return 1;
}

int hit_plane_within_box (const vox_dot origin, const vox_dot dir, const vox_dot planedot,
                          int planenum, vox_dot res, const vox_dot min, const vox_dot max)
{
    int i;
    float k;
    if (dir[planenum] == 0.0) return 0;
    k = planedot[planenum] - origin[planenum];
    if ((dir[planenum] < 0) != (k < 0)) return 0;

    k = k / dir[planenum];

    for (i=0; i<VOX_N; i++)
    {
        res[i] = origin[i] + k*dir[i];
        if ((res[i] < min[i]) || (res[i] > max[i])) return 0;
    }
    return 1;
}

int box_ball_interp (const vox_dot min, const vox_dot max, const vox_dot center, float radius)
{
    vox_dot fitted;
    fit_into_box (min, max, center, fitted);
    return (calc_sqr_metric (fitted, center) < (radius*radius)) ? 1 : 0;
}

// Not used anymore
#if 0
float* closest_in_set (vox_dot set[], int n, const vox_dot dot, float (*metric) (const vox_dot, const vox_dot))
{
    float *res = set[0];
    float dist = metric(dot, res);
    int i;

    for (i=1; i<n; i++)
    {
        if (dist > metric(set[i], dot))
        {
            dist = metric(set[i], dot);
            res = set[i];
        }
    }
    return res;
}
#endif
