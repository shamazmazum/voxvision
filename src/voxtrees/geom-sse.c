#include "geom.h"

#ifdef SSE_INTRIN
int voxel_in_box (const struct vox_box *box, const vox_dot dot)
{
    __v4sf d = _mm_load_ps (dot);
    __v4sf lt_min = d <  _mm_load_ps (box->min);
    __v4sf be_max = d >= _mm_load_ps (box->max);
    __v4sf outside = _mm_or_ps (lt_min, be_max);
    int mask = _mm_movemask_ps (outside);
    return !(mask & 7);
}

int fit_into_box (const struct vox_box *box, const vox_dot dot, vox_dot res)
{
    __v4sf v = _mm_load_ps (dot);
    __v4sf fitted = v;
    fitted = _mm_max_ps (fitted, _mm_load_ps (box->min));
    fitted = _mm_min_ps (fitted, _mm_load_ps (box->max));
    v = fitted != v;
    _mm_store_ps (res, fitted);
    return !(_mm_movemask_ps(v) & 7);
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
    __v4sf sub = _mm_load_ps(dot2) - _mm_load_ps(dot1);
    return (_mm_movemask_ps (sub) & 0x07);
}

int hit_box (const struct vox_box *box, const vox_dot origin, const vox_dot dir, vox_dot res)
{
    if (fit_into_box (box, origin, res)) return 1;

    __v4sf o = _mm_load_ps (origin);
    __v4sf d = _mm_load_ps (dir);
    __v4sf dist = (_mm_load_ps (res) - o) / d;

    // Get rid of NaNs (they can appear if d == 0)
    __v4sf mask = _mm_cmpunord_ps (dist, dist);
    __v4sf zeros = _mm_set_ps1 (0);
    __v4sf dist_nonans = _mm_blendv_ps (dist, zeros, mask);

    // Find the maximum distance
    __v4sf max1 = _mm_shuffle_ps (dist_nonans, dist_nonans, _MM_SHUFFLE (3, 1, 0, 2));
    __v4sf max2 = _mm_shuffle_ps (dist_nonans, dist_nonans, _MM_SHUFFLE (3, 0, 2, 1));
    max1 = _mm_max_ps (dist_nonans, max1);
    max2 = _mm_max_ps (dist_nonans, max2);
    max1 = _mm_max_ps (max1, max2);
    if (max1[0] <= 0) return 0;

    // Calculate hit point
    __v4sf hit = max1*d + o;
    __v4sf lt = hit < _mm_load_ps (box->min);
    __v4sf gt = hit > _mm_load_ps (box->max);
    __v4sf outside = _mm_or_ps (lt, gt);
    outside = _mm_andnot_ps(max1 == dist_nonans, outside);
    if (_mm_movemask_ps (outside) & 7) return 0;
    _mm_store_ps (res, hit);
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

    __v4sf o = _mm_load_ps (origin);
    __v4sf d = _mm_load_ps (dir);
    __v4sf kv = _mm_set_ps1 (k);
    __v4sf r = kv*d + o;
    __v4sf lt = r < _mm_load_ps (box->min);
    __v4sf gt = r > _mm_load_ps (box->max);
    __v4sf outside = _mm_or_ps (lt, gt);
    if (_mm_movemask_ps (outside) & 7) return 0;
    _mm_store_ps (res, r);
    return 1;
}
#endif
