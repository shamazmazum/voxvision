#include <math.h>
#include "geom.h"

#ifndef SSE_INTRIN
void sum_vector (const vox_dot a, const vox_dot b, vox_dot res)
{
    int i;
    for (i=0; i<VOX_N; i++) res[i] = a[i] + b[i];
}

int voxel_in_box (const struct vox_box *box, const vox_dot dot)
{
    int i;

    for (i=0; i<VOX_N; i++) {if ((dot[i] < box->min[i]) || (dot[i] >= box->max[i])) return 0;}
    return 1;
}

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

int get_subspace_idx (const vox_dot dot1, const vox_dot dot2)
{
    int res, i;
    res = 0;

    for (i=0; i<VOX_N; i++) res |= ((dot1[i] > dot2[i]) ? 1 : 0) << i;
    return res;
}

// Most of the following code is taken from C Graphics Gems
// See C Graphics Gems code for explanation
int hit_box (const struct vox_box *box, const vox_dot origin, const vox_dot dir, vox_dot res)
{
    float max_dist, tmp;
    int i, plane_num;
    if (fit_into_box (box, origin, res)) return 1;

    plane_num = 0;
    max_dist = -1;
    for (i=0; i<VOX_N; i++)
    {
        tmp = (res[i] - origin[i])/dir[i];
        if (tmp > max_dist)
        {
            plane_num = i;
            max_dist = tmp;
        }
    }
    if (max_dist <= 0) return 0;

    for (i=0; i<VOX_N; i++)
    {
        if (i != plane_num)
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
    k = planedot[planenum] - origin[planenum];
    /*
      k == 0 means that origin lays on the plane.
      This is a special case which is not handeled here.
      Just return that there is no intersection.
    */
    if ((k == 0) ||
        (dir[planenum] < 0) != (k < 0)) return 0;

    k = k / dir[planenum];

    for (i=0; i<VOX_N; i++)
    {
        res[i] = origin[i] + k*dir[i];
        if ((res[i] < box->min[i]) || (res[i] > box->max[i])) return 0;
    }
    return 1;
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

int box_ball_interp (const struct vox_box *box, const vox_dot center, float radius)
{
    vox_dot fitted;
    fit_into_box (box, center, fitted);
    return calc_sqr_metric (fitted, center) < (radius*radius);
}
#endif /* SSE_INTRIN */

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
    return fabsf (n*vox_volume - bb_volume) < vox_volume;
}

int divide_box (const struct vox_box *box, const vox_dot center, struct vox_box *res, int idx)
{
    int i;

    vox_box_copy (res, box);

    for (i=0; i<VOX_N; i++)
    {
        if (idx & (1<<i)) res->max[i] = center[i];
        else res->min[i] = center[i];
        if (res->min[i] == res->max[i]) return 0;
    }
    return 1;
}

/*
  XXX: Use of this function is discouraged.
  This function is used only for dense leafs to get dimensions and number of
  voxels inside those leafs. If the precision of floating point arithmetic is
  not enough, this function will return wrong results. Further work must be done
  to eliminate use of this function (and rework flatten_tree() so it doesn't
  depend on this one).
*/
void get_dimensions (const struct vox_box *box, size_t dim[])
{
    int i;
    for (i=0; i<VOX_N; i++) dim[i] = (box->max[i] - box->min[i]) / vox_voxel[i];
}

// XXX: Depends on get_dimensions
int stripep (const struct vox_box *box, int *which)
{
    size_t dim[VOX_N];
    int i, count=0;
    get_dimensions (box, dim);
    for (i=0; i<VOX_N; i++)
    {
        if (dim[i] == 1) count++;
        else *which = i;
    }
    return count == (VOX_N-1);
}
