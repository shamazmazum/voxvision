#include <math.h>
#include "geom.h"

static int dotall (cmpmask m) {
    return m.x && m.y && m.z && m.w;
}

static int dotsome (cmpmask m) {
    return m.x || m.y || m.z || m.w;
}

int voxel_in_box (const struct vox_box *box, const vox_dot dot) {
    cmpmask m1 = dot >= box->min;
    cmpmask m2 = dot <  box->max;

    // TODO: Check if we can replace < and >= with -. Will clang
    // recognize this pattern?
    return dotall(m1) && dotall(m2);
}

static vox_dot fit_into_box (const struct vox_box *box, const vox_dot dot) {
    vox_dot t1 = vmin (dot, box->max);
    vox_dot t2 = vmax (t1,  box->min);
    return t2;
}

vox_dot closest_vertex (const struct vox_box *box, const vox_dot dot) {
    vox_dot d1 = vabs(box->min - dot);
    vox_dot d2 = vabs(box->max - dot);
    cmpmask m = d1 < d2;
    return m ? box->min : box->max;
}

static int dot_mask_idx (const vox_dot x) {
    int b0 = x[0] < 0;
    int b1 = x[1] < 0;
    int b2 = x[2] < 0;
    int b3 = x[3] < 0;

    return (b0 << 0) | (b1 << 1) | (b2 << 2) | (b3 << 3);
}

int get_subspace_idx (const vox_dot center, const vox_dot dot) {
    return dot_mask_idx (dot - center);
}

int get_corrected_subspace_idx (const vox_dot center,
                                const vox_dot dot,
                                const vox_dot direction) {
    vox_dot diff = dot - center;
    cmpmask c = center == dot;
    vox_dot x = c ? direction : diff;
    return dot_mask_idx (x);
}

// Most of the following code is taken from C Graphics Gems
// See C Graphics Gems code for explanation
int hit_box (const struct vox_box *box,
             const vox_dot         origin,
             const vox_dot         dir,
             vox_dot              *res) {
    vox_dot fit = fit_into_box (box, origin);
    vox_dot sub = fit - origin;
    vox_dot zero = (vox_dot){0, 0, 0, 0};

    if (dotall(sub == zero)) {
        *res = origin;
        return 1;
    }

    // Replace infinities (they can appear if d == 0) with zeros
    cmpmask mask = dir == zero;
    vox_dot dist = mask ? zero : sub / dir;

    vox_dot max1 = dist.zxyw;
    vox_dot max2 = dist.yzxw;
    max1 = vmax (dist, max1);
    max2 = vmax (dist, max2);
    max1 = vmax (max1, max2);

    if (max1.x <= 0) {
        return 0;
    }

    vox_dot hit = (max1 == dist) ? fit : (max1 * dir + origin);

    cmpmask lt = hit < box->min;
    cmpmask gt = hit > box->max;

    // TODO: Check if we can replace < and > with -. Will clang
    // recognize this pattern?
    if (dotsome (lt) || dotsome (gt)) {
        return 0;
    }

    *res = hit;
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

float vox_abs_metric (const vox_dot dot1, const vox_dot dot2)
{
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += fabsf (dot1[i] - dot2[i]);
    return res;
}

float vox_sqr_metric (const vox_dot dot1, const vox_dot dot2)
{
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += powf (dot1[i] - dot2[i], 2.0);
    return res;
}

float vox_sqr_norm (const vox_dot dot)
{
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += powf (dot[i], 2.0);
    return res;
}

#if 0
int box_ball_interp (const struct vox_box *box, const vox_dot center, float radius)
{
    vox_dot fitted;
    fit_into_box (box, center, fitted);
    return vox_sqr_metric (fitted, center) < (radius*radius);
}
#endif

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
