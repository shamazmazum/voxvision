#include <math.h>
#include "geom.h"

#ifdef SSE_INTRIN
void sum_vector (const vox_dot a, const vox_dot b, vox_dot res)
{
    __v4sf dot = _mm_load_ps (a);
    _mm_store_ps (res, dot + _mm_load_ps (b));
}
#else
void sum_vector (const vox_dot a, const vox_dot b, vox_dot res)
{
    int i;
    for (i=0; i<VOX_N; i++) res[i] = a[i] + b[i];
}
#endif

int get_subspace_idx (const vox_dot dot1, const vox_dot dot2)
{
    int res, i;
    res = 0;

    for (i=0; i<VOX_N; i++) res |= ((dot1[i] > dot2[i]) ? 1 : 0) << i;
    return res;
}

float calc_abs_metric (const vox_dot dot1, const vox_dot dot2)
{
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += fabsf (dot1[i] - dot2[i]);
    return res;
}

float calc_sqr_metric (const vox_dot dot1, const vox_dot dot2)
{
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += powf (dot1[i] - dot2[i], 2.0);
    return res;
}

#ifdef SSE_INTRIN
static int fit_into_box (const struct vox_box *box, const vox_dot dot, vox_dot res)
{
    __v4sf v = _mm_load_ps (dot);
    __v4sf fitted = v;
    fitted = _mm_max_ps (fitted, _mm_load_ps (box->min));
    fitted = _mm_min_ps (fitted, _mm_load_ps (box->max));
    v = fitted == v;
    _mm_store_ps (res, fitted);
    return (_mm_movemask_ps(v) & 7) == 7;
}
#else
static int fit_into_box (const struct vox_box *box, const vox_dot dot, vox_dot res)
{
    int i, the_same = 1;
    for (i=0; i<VOX_N; i++)
    {
        if (dot[i] < box->min[i])
        {
            res[i] = box->min[i];
            the_same = 0;
        }
        else if (dot[i] > box->max[i])
        {
            res[i] = box->max[i];
            the_same = 0;
        }
        else res[i] = dot[i];
    }
    return the_same;
}
#endif

// Most of the following code is taken from C Graphics Gems
// See C Graphics Gems code for explanation
int hit_box (const struct vox_box *box, const vox_dot origin, const vox_dot dir, vox_dot res)
{
    vox_dot candidate_plane;
    float max_dist, tmp;
    int i, plane_num;
    int insidep = fit_into_box (box, origin, candidate_plane);
    if (insidep)
    {
        vox_dot_copy (res, origin);
        return 1;
    }

    plane_num = 0;
    max_dist = -1;
    for (i=0; i<VOX_N; i++)
    {
        if ((candidate_plane[i] == origin[i]) || dir[i] == 0.0) tmp = -1.0;
        else
        {
            tmp = (candidate_plane[i] - origin[i])/dir[i];
            if (tmp > max_dist)
            {
                plane_num = i;
                max_dist = tmp;
            }
        }
    }
    if (max_dist < 0) return 0;

    for (i=0; i<VOX_N; i++)
    {
        if (i==plane_num) res[i] = candidate_plane[i];
        else
        {
            tmp = origin[i] + max_dist*dir[i];
            if ((tmp < box->min[i]) || (tmp > box->max[i])) return 0;
            res[i] = tmp;
        }
    }
    
    return 1;
}

int hit_plane_within_box (const vox_dot origin, const vox_dot dir, const vox_dot planedot,
                          int planenum, vox_dot res, const struct vox_box *box)
{
    int i;
    float k;
    if (dir[planenum] == 0.0) return 0;
    k = planedot[planenum] - origin[planenum];
    /*
      k == 0 means that origin lays on the plane.
      This is a special case which is not handeled here.
      Just return that there is no intersection.
    */
    if (k == 0) return 0;
    if ((dir[planenum] < 0) != (k < 0)) return 0;

    k = k / dir[planenum];

    for (i=0; i<VOX_N; i++)
    {
        res[i] = origin[i] + k*dir[i];
        if ((res[i] < box->min[i]) || (res[i] > box->max[i])) return 0;
    }
    return 1;
}

int box_ball_interp (const struct vox_box *box, const vox_dot center, float radius)
{
    vox_dot fitted;
    fit_into_box (box, center, fitted);
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

float fill_ratio (const struct vox_box *box, size_t n)
{
    float bb_volume, vox_volume;
    vox_dot size;
    int i;

    for (i=0; i<VOX_N; i++) size[i] = box->max[i] - box->min[i];
    bb_volume = size[0]*size[1]*size[2];
    vox_volume = vox_voxel[0]*vox_voxel[1]*vox_voxel[2];
    vox_volume *= n;
    return vox_volume/bb_volume;
}

int dense_set_p (const struct vox_box *box, size_t n)
{
    float bb_volume, vox_volume;
    vox_dot size;
    int i;

    for (i=0; i<VOX_N; i++) size[i] = box->max[i] - box->min[i];
    bb_volume = size[0]*size[1]*size[2];
    vox_volume = vox_voxel[0]*vox_voxel[1]*vox_voxel[2];
    return fabs (n*vox_volume - bb_volume) < vox_volume;
}

// SSE counterpart? Is it worth it?
int voxel_in_box (const struct vox_box *box, const vox_dot dot)
{
    int i;

    for (i=0; i<VOX_N; i++) {if ((dot[i] < box->min[i]) || (dot[i] >= box->max[i])) return 0;}
    return 1;
}

void closest_vertex (const struct vox_box *box, const vox_dot dot, vox_dot res)
{
    int i;
    for (i=0; i<VOX_N; i++)
    {
        float d1 = fabsf (box->min[i] - dot[i]);
        float d2 = fabsf (box->max[i] - dot[i]);
        res[i] = (d1 < d2) ? box->min[i] : box->max[i];
    }
}
