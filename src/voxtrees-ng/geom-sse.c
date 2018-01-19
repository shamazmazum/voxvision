#include "geom.h"
#include "tree.h"

#ifdef SSE_INTRIN

float vox_abs_metric (const vox_dot d1, const vox_dot d2)
{
    __v4sf dot1 = _mm_load_ps (d1);
    __v4sf dot2 = _mm_load_ps (d2);
    __v4sf diff = dot1 - dot2;
    __v4sf negzero = -_mm_set1_ps (0);

    __v4sf res = _mm_andnot_ps (negzero, diff);
    return res[0] + res[1] + res[2];
}

float vox_sqr_metric (const vox_dot d1, const vox_dot d2)
{
    __v4sf dot1 = _mm_load_ps (d1);
    __v4sf dot2 = _mm_load_ps (d2);
    __v4sf diff = dot1 - dot2;

    __v4sf res = diff * diff;
    return res[0] + res[1] + res[2];
}

int subspace_idx (const vox_dot center, const vox_dot dot)
{
    __v4sf c = _mm_load_ps (center);
    __v4sf d = _mm_load_ps (dot);

    __v4sf mask = d >= c;
    return _mm_movemask_ps (mask) & 0x7;
}

int corrected_subspace_idx (const vox_dot center, const vox_dot dot, const vox_dot direction)
{
    __v4sf c = _mm_load_ps (center);
    __v4sf d = _mm_load_ps (dot);
    __v4sf dir = _mm_load_ps (direction);

    __v4sf idx1 = d > c;
    __v4sf idx2 = dir > _mm_set1_ps (0);
    __v4sf idx = _mm_blendv_ps (idx1, idx2, c == d);

    return _mm_movemask_ps (idx) & 0x7;
}

static __v4sf fit_into_box (const struct vox_box *box, __v4sf dot)
{
    __v4sf res = _mm_min_ps (dot, _mm_load_ps (box->max));
    res = _mm_max_ps (res, _mm_load_ps (box->min));

    return res;
}

static int hit_box_vect (const struct vox_box *box, __v4sf origin,
             __v4sf direction, vox_dot res)
{
    __v4sf fit = fit_into_box (box, origin);
    if (!(_mm_movemask_ps (origin != fit) & 0x7))
    {
        _mm_store_ps (res, origin);
        return 1;
    }

    __v4sf sub = fit - origin;
    __v4sf mask = direction == _mm_set_ps1(0);

    // Replace NaNs (they can appear if d == 0) with zeros
    __v4sf dist = _mm_blendv_ps (sub / direction, _mm_set_ps1(0), mask);

    // Find the maximum distance
    __v4sf max1 = _mm_shuffle_ps (dist, dist, _MM_SHUFFLE (3, 1, 0, 2));
    __v4sf max2 = _mm_shuffle_ps (dist, dist, _MM_SHUFFLE (3, 0, 2, 1));
    max1 = _mm_max_ps (dist, max1);
    max2 = _mm_max_ps (dist, max2);
    max1 = _mm_max_ps (max1, max2);
    if (max1[0] <= 0) return 0;

    // Calculate hit point
    __v4sf hit = _mm_blendv_ps (max1 * direction + origin, fit, max1 == dist);
    __v4sf lt = hit < _mm_load_ps (box->min);
    __v4sf gt = hit > _mm_load_ps (box->max);
    __v4sf outside = _mm_or_ps (lt, gt);
    if (_mm_movemask_ps (outside) & 0x7) return 0;
    _mm_store_ps (res, hit);
    return 1;
}

int hit_box (const struct vox_box *box, const vox_dot origin,
             const vox_dot direction, vox_dot res)
{
    return hit_box_vect (box, _mm_load_ps (origin), _mm_load_ps (direction), res);
}

int hit_box_outer (const struct vox_box *box, const vox_dot origin,
                   const vox_dot direction, vox_dot res)
{
    __v4sf old_origin = _mm_load_ps (origin);
    __v4sf old_direction = _mm_load_ps (direction);
    __v4sf metric = _mm_set1_ps (vox_sqr_metric (box->min, box->max));

    __v4sf new_origin = old_origin + metric * old_direction;
    __v4sf new_direction = - old_direction;

    return hit_box_vect (box, new_origin, new_direction, res);
}

int hit_plane_within_box (const vox_dot origin, const vox_dot dir, const vox_dot planedot,
                          int planenum, vox_dot res, const struct vox_box *box)
{
    float k;

    if (dir[planenum] == 0) return 0;

    k = (planedot[planenum] - origin[planenum]) / dir[planenum];
    /*
      k == 0 means that origin lays on the plane.
      This is a special case which is not handeled here.
      Just return that there is no intersection.
    */
    if ((k == 0) || (k < 0)) return 0;

    __v4sf o = _mm_load_ps (origin);
    __v4sf d = _mm_load_ps (dir);
    __v4sf kv = _mm_set_ps1 (k);
    __v4sf r = kv*d + o;
    __v4sf lt = r < _mm_load_ps (box->min);
    __v4sf gt = r > _mm_load_ps (box->max);
    __v4sf outside = _mm_or_ps (lt, gt);
    if (_mm_movemask_ps (outside) & 0x7) return 0;
    _mm_store_ps (res, r);
    return 1;
}

int dot_inside_box (const struct vox_box *box, const vox_dot dot, int strong)
{
    __v4sf mask1, mask2, mask;
    __v4sf min = _mm_load_ps (box->min);
    __v4sf max = _mm_load_ps (box->max);
    __v4sf d = _mm_load_ps (dot);

    mask1 = (strong)? (d <= min): (d < min);
    mask2 = (strong)? (d >= max): (d > max);

    mask = _mm_or_ps (mask1, mask2);
    return !(_mm_movemask_ps (mask) & 0x7);
}

#endif
