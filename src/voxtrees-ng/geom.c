#include <math.h>
#include "geom.h"
#include "tree.h"

int dot_inside_box (const struct vox_box *box, const vox_dot dot, int strong)
{
    int i;
    for (i=0; i<3; i++) {
        if (strong) {
            if (dot[i] <= box->min[i] || dot[i] >= box->max[i]) return 0;
        } else {
            if (dot[i] < box->min[i] || dot[i] > box->max[i]) return 0;
        }
    }
    return 1;
}

int box_inside_box (const struct vox_box *outer, const struct vox_box *inner, int strong)
{
    return dot_inside_box (outer, inner->min, strong) &&
        dot_inside_box (outer, inner->max, strong);
}

int voxel_inside_box (const struct vox_box *box, const vox_dot voxel, int strong)
{
    vox_dot tmp;
    vox_dot_add (voxel, vox_voxel, tmp);
    return dot_inside_box (box, voxel, strong) &&
        dot_inside_box (box, tmp, strong);
}

void subspace_box (const struct vox_box *space, const vox_dot center,
                   struct vox_box *subspace, int subspace_idx)
{
    vox_box_copy (subspace, space);
    int i;
    for (i=0; i<3; i++) {
        if (subspace_idx & (1 << i))
            subspace->min[i] = center[i];
        else subspace->max[i] = center[i];
    }
}

int subspace_idx (const vox_dot center, const vox_dot dot)
{
    int i, idx = 0;
    for (i=0; i<3; i++)
        idx |= (dot[i] >= center[i])? (1 << i): 0;
    return idx;
}

float squared_metric (const vox_dot d1, const vox_dot d2)
{
    float tmp, res = 0;
    int i;

    for (i=0; i<3; i++) {
        tmp = d2[i] - d1[i];
        res += tmp*tmp;
    }
    return res;
}

static int fit_into_box (const struct vox_box *box, const vox_dot dot, vox_dot res)
{
    int i, the_same = 1;
    for (i=0; i<3; i++)
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

int hit_box (const struct vox_box *box, const vox_dot origin,
             const vox_dot direction, vox_dot res)
{
    float max_dist, tmp;
    int i, plane_num;
    if (fit_into_box (box, origin, res)) return 1;

    plane_num = 0;
    max_dist = -1;
    for (i=0; i<3; i++)
    {
        tmp = (res[i] - origin[i])/direction[i];
        if (tmp > max_dist)
        {
            plane_num = i;
            max_dist = tmp;
        }
    }
    if (max_dist <= 0) return 0;

    for (i=0; i<3; i++)
    {
        if (i != plane_num)
        {
            tmp = origin[i] + max_dist*direction[i];
            if ((tmp < box->min[i]) || (tmp > box->max[i])) return 0;
            res[i] = tmp;
        }
    }

    return 1;
}

int hit_box_outer (const struct vox_box *box, const vox_dot origin,
                   const vox_dot direction, vox_dot res)
{
    /*
     * Actually implement this with help of hit_box.
     * We calculate new origin' = origin + squared_metric(box)*dir and invert dir, then
     * call hit_box.
     */
    vox_dot new_origin, new_direction;
    int i;
    float sm = squared_metric (box->min, box->max);

    for (i=0; i<3; i++) {
        new_origin[i] = origin[i] + sm*direction[i];
        new_direction[i] = -direction[i];
    }
    return hit_box (box, new_origin, new_direction, res);
}

void voxel_align (vox_dot dot)
{
    int i;
    for (i=0; i<3; i++)
        dot[i] = vox_voxel[i] * floorf (dot[i] / vox_voxel[i]);
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

    for (i=0; i<3; i++)
    {
        res[i] = origin[i] + k*dir[i];
        if ((res[i] < box->min[i]) || (res[i] > box->max[i])) return 0;
    }
    return 1;
}
