#include <stdlib.h>
#include <math.h>
#ifdef __x86_64__
#include <xmmintrin.h>
#endif
#include "geom.h"

typedef int int4 __attribute__((ext_vector_type(4)));
#ifdef __x86_64__
static unsigned int extract_mask (int4 m) {
    return _mm_movemask_ps ((__m128i)m) & 7;
}
#else
static unsigned int extract_mask (int4 m) {
    return ((m.x != 0) << 0) | ((m.y != 0) << 1) | ((m.z != 0) << 2);
}
#endif

static vox_dot box_clamp (const struct vox_box *box, const vox_dot dot) {
    vox_dot clamped;
    clamped = __builtin_elementwise_min (box->max, dot);
    clamped = __builtin_elementwise_max (box->min, clamped);
    return clamped;
}

int voxel_in_box (const struct vox_box *box, const vox_dot dot) {
    vox_dot clamped = box_clamp (box, dot);
    return extract_mask (dot == clamped) == 7;
}

unsigned int get_subspace_idx (const vox_dot center, const vox_dot dot) {
    unsigned int res, i;
    res = 0;

    for (i=0; i<VOX_N; i++) {
        res |= (center[i] > dot[i]) << i;
    }
    return res;
}

unsigned int get_corrected_subspace_idx (const vox_dot center,
                                         const vox_dot dot,
                                         const vox_dot direction) {
    auto mask1 = direction < 0;
    auto mask2 = center > dot;
    auto mask3 = (center == dot) ? mask1 : mask2;

    return extract_mask (mask3);
}

// Most of the following code is taken from C Graphics Gems
// See C Graphics Gems code for explanation
int vox_hit_box (const struct vox_box *box,
                 const vox_dot origin,
                 const vox_dot dir,
                 vox_dot *resptr) {
    float max_dist, tmp;
    int i, plane_num;
    vox_dot res;

    res = box_clamp (box, origin);
    if (extract_mask (res == origin) == 7) {
        *resptr = res;
        return 1;
    }

    plane_num = 0;
    max_dist = -1;
    for (i=0; i<VOX_N; i++) {
        tmp = (res[i] - origin[i])/dir[i];
        if (tmp > max_dist) {
            plane_num = i;
            max_dist = tmp;
        }
    }

    if (max_dist <= 0) {
        return 0;
    }

    for (i=0; i<VOX_N; i++) {
        if (i != plane_num) {
            tmp = origin[i] + max_dist*dir[i];
            if ((tmp < box->min[i]) || (tmp > box->max[i])) {
                return 0;
            }
            res[i] = tmp;
        }
    }

    *resptr = res;
    return 1;
}

int hit_plane_within_box (const vox_dot origin, const vox_dot dir, const vox_dot planedot,
                          int planenum, vox_dot *resptr, const struct vox_box *box) {
    vox_dot res;
    int i;
    float k;
    k = planedot[planenum] - origin[planenum];
    /*
      k == 0 means that origin lays on the plane.
      This is a special case which is not handeled here.
      Just return that there is no intersection.
    */
    if ((k == 0) || (dir[planenum] < 0) != (k < 0)) {
        return 0;
    }

    k = k / dir[planenum];

    for (i=0; i<VOX_N; i++) {
        res[i] = origin[i] + k*dir[i];
        if ((res[i] < box->min[i]) || (res[i] > box->max[i])) return 0;
    }

    *resptr = res;
    return 1;
}

float vox_abs_metric (const vox_dot dot1, const vox_dot dot2) {
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) {
        res += fabsf (dot1[i] - dot2[i]);
    }
    return res;
}

float vox_sqr_metric (const vox_dot dot1, const vox_dot dot2) {
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) {
        res += powf (dot1[i] - dot2[i], 2.0);
    }
    return res;
}

float vox_sqr_norm (const vox_dot dot) {
    int i;
    float res = 0;
    for (i=0; i<VOX_N; i++) res += powf (dot[i], 2.0);
    return res;
}

int vox_box_ball_interp (const struct vox_box *box, const vox_dot center, float radius) {
    vox_dot clamped = box_clamp (box, center);
    return vox_sqr_metric (clamped, center) < (radius*radius);
}

int dense_set_p (const struct vox_box *box, size_t n) {
    float bb_volume, vox_volume;
    vox_dot size;
    int i;

    for (i=0; i<VOX_N; i++) {
        size[i] = box->max[i] - box->min[i];
    }
    bb_volume = size[0]*size[1]*size[2];
    vox_volume = vox_voxel[0]*vox_voxel[1]*vox_voxel[2];
    return fabsf (n*vox_volume - bb_volume) < vox_volume;
}
