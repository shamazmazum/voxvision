#include "geom.h"

#ifdef SSE_INTRIN
static float vox_abs_metric_ (__v4sf dot1, __v4sf dot2)
{
    __v4sf diff = dot1 - dot2;
    diff = _mm_and_ps (diff, _mm_set_epi32 (0,          0x7fffffff,
                                            0x7fffffff, 0x7fffffff));
    return diff[0] + diff[1] + diff[2];
}

static float vox_sqr_metric_ (__v4sf dot1, __v4sf dot2)
{
    __v4sf diff = dot1 - dot2;
    diff *= diff;
    return diff[0] + diff[1] + diff[2];
}

static float vox_sqr_norm_ (__v4sf dot)
{
    dot *= dot;
    return dot[0] + dot[1] + dot[2];
}

static int mask_bits_clear (__v4sf mask)
{
    return !(_mm_movemask_ps (mask) & 0x07);
}
static int mask_bits_set (__v4sf mask)
{
    return _mm_movemask_ps (mask) & 0x07;
}

static __v4sf fit_into_box (const struct vox_box *box, __v4sf dot)
{
    __v4sf fit;
    fit = _mm_max_ps (dot, _mm_load_ps (box->min));
    fit = _mm_min_ps (fit, _mm_load_ps (box->max));
    return fit;
}

int voxel_in_box (const struct vox_box *box, const vox_dot dot)
{
    __v4sf d = _mm_load_ps (dot);
    __v4sf lt_min = d <  _mm_load_ps (box->min);
    __v4sf be_max = d >= _mm_load_ps (box->max);
    __v4sf outside = _mm_or_ps (lt_min, be_max);
    return mask_bits_clear (outside);
}

void closest_vertex (const struct vox_box *box, const vox_dot dot, vox_dot res)
{
    __v4sf min = _mm_load_ps (box->min);
    __v4sf max = _mm_load_ps (box->max);
    __v4sf d = _mm_load_ps (dot);
    __v4sf mask = _mm_set1_epi32 (0x7fffffff);
    __v4sf d1 = _mm_and_ps (min - d, mask);
    __v4sf d2 = _mm_and_ps (max - d, mask);
    __v4sf cmp = d1 < d2;
    __v4sf r = _mm_blendv_ps (max, min, cmp);
    _mm_store_ps (res, r);
}

int get_subspace_idx (const vox_dot dot1, const vox_dot dot2)
{
    __v4sf sub = _mm_load_ps(dot2) < _mm_load_ps(dot1);
    return mask_bits_set (sub);
}

int hit_box (const struct vox_box *box, const vox_dot origin, const vox_dot dir, vox_dot res)
{
    __v4sf o = _mm_load_ps (origin);
    __v4sf fit = fit_into_box (box, o);
    __v4sf sub = fit - o;
    if (mask_bits_clear (sub != _mm_set_ps1(0)))
    {
        _mm_store_ps (res, o);
        return 1;
    }
    __v4sf d = _mm_load_ps (dir);
    __v4sf mask = d == _mm_set_ps1(0);

    // Replace NaNs (they can appear if d == 0) with zeros
    __v4sf dist = _mm_blendv_ps (sub / d, _mm_set_ps1(0), mask);

    // Find the maximum distance
    __v4sf max1 = _mm_shuffle_ps (dist, dist, _MM_SHUFFLE (3, 1, 0, 2));
    __v4sf max2 = _mm_shuffle_ps (dist, dist, _MM_SHUFFLE (3, 0, 2, 1));
    max1 = _mm_max_ps (dist, max1);
    max2 = _mm_max_ps (dist, max2);
    max1 = _mm_max_ps (max1, max2);
    if (max1[0] <= 0) return 0;

    // Calculate hit point
    __v4sf hit = _mm_blendv_ps (max1 * d + o, fit, max1 == dist);
    __v4sf lt = hit < _mm_load_ps (box->min);
    __v4sf gt = hit > _mm_load_ps (box->max);
    __v4sf outside = _mm_or_ps (lt, gt);
    if (mask_bits_set (outside)) return 0;
    _mm_store_ps (res, hit);
    return 1;
}

int hit_plane_within_box (const vox_dot origin, const vox_dot dir, const vox_dot planedot,
                          int planenum, vox_dot res, const struct vox_box *box)
{
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

    __v4sf o = _mm_load_ps (origin);
    __v4sf d = _mm_load_ps (dir);
    __v4sf kv = _mm_set_ps1 (k);
    __v4sf r = kv*d + o;
    __v4sf lt = r < _mm_load_ps (box->min);
    __v4sf gt = r > _mm_load_ps (box->max);
    __v4sf outside = _mm_or_ps (lt, gt);
    if (mask_bits_set (outside)) return 0;
    _mm_store_ps (res, r);
    return 1;
}

int box_ball_interp (const struct vox_box *box, const vox_dot center, float radius)
{
    __v4sf c = _mm_load_ps (center);
    __v4sf fit = fit_into_box (box, c);
    return vox_sqr_metric_ (fit, c) < (radius*radius);
}

float vox_sqr_metric (const vox_dot dot1, const vox_dot dot2)
{
    return vox_sqr_metric_ (_mm_load_ps (dot1), _mm_load_ps (dot2));
}

float vox_abs_metric (const vox_dot dot1, const vox_dot dot2)
{
    return vox_abs_metric_ (_mm_load_ps (dot1), _mm_load_ps (dot2));
}

float vox_sqr_norm (const vox_dot dot)
{
    return vox_sqr_norm_ (_mm_load_ps (dot));
}
#endif
