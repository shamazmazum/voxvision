#include <math.h>
#include <string.h>

#include "geom.h"
#include "params.h"

#ifdef SSE_ENABLE_SEARCH
#include <xmmintrin.h>
#include <stdlib.h>
#define SSE_CMP_MASK ((1<<VOX_N) - 1)
#endif

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

#ifdef SSE_ENABLE_SEARCH
static inline int fit_and_betweenp (const vox_dot min, const vox_dot max, const vox_dot dot, vox_dot res)
{
    __v4sf dotv = _mm_load_ps (dot);
    __v4sf fittedv;

    fittedv = __builtin_ia32_maxps (dotv,    _mm_load_ps (min));
    fittedv = __builtin_ia32_minps (fittedv, _mm_load_ps (max));
    if (res != NULL) _mm_store_ps (res, fittedv);

    __v4sf eqv = fittedv == dotv;
    return (__builtin_ia32_movmskps (eqv) & SSE_CMP_MASK) == SSE_CMP_MASK;
}

int fit_into_box (const vox_dot min, const vox_dot max, const vox_dot dot, vox_dot res)
{
    return fit_and_betweenp (min, max, dot, res);
}

int dot_betweenp (const vox_dot min, const vox_dot max, const vox_dot dot)
{
    return fit_and_betweenp (min, max, dot, NULL);
}

#else /* SSE_ENABLE_SEARCH */
int fit_into_box (const vox_dot min, const vox_dot max, const vox_dot dot, vox_dot res)
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

int dot_betweenp (const vox_dot min, const vox_dot max, const vox_dot dot)
{
    vox_uint i;

    for (i=0; i<VOX_N; i++) {if ((dot[i] < min[i]) || (dot[i] > max[i])) return 0;}
    return 1;
}
#endif /* SSE_ENABLE_SEARCH */

// Most of the following code is taken from C Graphics Gems
// See C Graphics Gems code for explanation
int hit_box (const vox_dot min, const vox_dot max, const vox_dot origin, const vox_dot dir, vox_dot res)
{
    vox_dot candidate_plane, tdist;
    float max_dist;
    int i, plane_num;
    int insidep = fit_into_box (min, max, origin, candidate_plane);
    if (insidep)
    {
        memcpy (res, origin, sizeof(vox_dot));
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

int hit_plane (const vox_dot origin, const vox_dot dir, const vox_dot planedot, int planenum, vox_dot res)
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

int box_ball_interp (const vox_dot min, const vox_dot max, const vox_dot center, float radius)
{
    vox_dot fitted;
    fit_into_box (min, max, center, fitted);
    return (calc_sqr_metric (fitted, center) < (radius*radius)) ? 1 : 0;
}

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
