#include <math.h>
#include <string.h>

#include "geom.h"
#include "params.h"

float *sum_vector (const float *a, const float *b, float *res)
{
    int i;
    for (i=0; i<VOX_N; i++) res[i] = a[i] + b[i];
    return res;
}

uint8_t get_subspace_idx (const float *dot1, const float *dot2)
{
    uint8_t res = 0;
    int i;

    for (i=0; i<VOX_N; i++) res |= ((dot1[i] > dot2[i]) ? 1 : 0) << i;
    return res;
}

float calc_abs_metric (const float *dot1, const float *dot2)
{
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += fabsf (dot1[i] - dot2[i]);
    return res;
}

float calc_sqr_metric (const float *dot1, const float *dot2)
{
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += powf (dot1[i] - dot2[i], 2.0);
    return res;
}

int fit_into_box (const float *min, const float *max, const float *dot, float *res)
{
    int i;
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

int dot_betweenp (const float *min, const float *max, const float *dot)
{
    int i;

    for (i=0; i<VOX_N; i++) {if ((dot[i] < min[i]) || (dot[i] > max[i])) return 0;}
    return 1;
}

// Most of the following code is taken from C Graphics Gems
// See C Graphics Gems code for explanation
int hit_box (const float *min, const float *max, const float *origin, const float *dir, float *res)
{
    float candidate_plane[VOX_N];
    float tdist[VOX_N];
    float max_dist;
    int i, plane_num;
    int insidep = fit_into_box (min, max, origin, candidate_plane);
    if (insidep)
    {
        memcpy (res, origin, sizeof(float)*3);
        return 1;
    }

    for (i=0; i<VOX_N; i++)
    {
        if ((candidate_plane[i] == origin[i]) || dir[i] == 0.0) tdist[i] = -1.0;
        else tdist[i] = (candidate_plane[i] - origin[i])/dir[i];
    }

    plane_num = 0; max_dist = tdist[0];
    for (i=1; i<VOX_N; i++)
    {
        if (max_dist < tdist[i])
        {
            plane_num = i;
            max_dist = tdist[i];
        }
    }

    if (max_dist < 0) return 0;

    for (i=0; i<VOX_N; i++)
    {
        if (i==plane_num) res[i] = candidate_plane[i];
        else res[i] = origin[i] + max_dist*dir[i];
    }
    
    if (dot_betweenp (min, max, res)) return 1;
    else return 0;
}

int hit_plane (const float *origin, const float *dir, const float *planedot, int planenum, float *res)
{
    int i;
    if (dir[planenum] == 0.0) return 0;

    float k = (planedot[planenum] - origin[planenum]) / dir[planenum];
    if (k < 0.0) return 0;

    for (i=0; i<VOX_N; i++)
    {
        if (i == planenum) res[i] = planedot[i];
        else res[i] = origin[i] + k*dir[i];
    }
    return 1;
}

int box_ball_interp (const float *min, const float *max, const float *center, float radius)
{
    float fitted[VOX_N];
    fit_into_box (min, max, center, fitted);
    return (calc_sqr_metric (fitted, center) < (radius*radius)) ? 1 : 0;
}

float* closest_in_set (float set[][VOX_N], int n, const float *dot, float (*metric) (const float*, const float*))
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
