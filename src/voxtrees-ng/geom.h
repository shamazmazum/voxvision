#ifndef _GEOM_H_
#define _GEOM_H_
#include "types.h"

#define vox_dot_add(a,b,res) do { \
        res[0] = a[0] + b[0];     \
        res[1] = a[1] + b[1];     \
        res[2] = a[2] + b[2];     \
    }                             \
    while (0);

#define vox_dot_sub(a,b,res) do { \
        res[0] = a[0] - b[0];     \
        res[1] = a[1] - b[1];     \
        res[2] = a[2] - b[2];     \
    }                             \
    while (0);

#define vox_dot_mul(a,b,res) do { \
        res[0] = a[0] * b[0];     \
        res[1] = a[1] * b[1];     \
        res[2] = a[2] * b[2];     \
    }                             \
    while (0);

float vox_abs_metric (const vox_dot d1, const vox_dot d2);
float vox_sqr_metric (const vox_dot d1, const vox_dot d2);
#ifdef VOXTREES_NG_SOURCE
int dot_inside_box (const struct vox_box *box, const vox_dot dot, int strong);
int box_inside_box (const struct vox_box *outer, const struct vox_box *inner, int strong);
int voxel_inside_box (const struct vox_box *box, const vox_dot voxel, int strong);

void subspace_box (const struct vox_box *space, const vox_dot center,
                   struct vox_box *subspace, int subspace_idx);
int subspace_idx (const vox_dot center, const vox_dot dot);

/*
 * Generally this works like get_subspace_idx () but corrects subspace
 * index if center[i] == dot[i]. In this case, direction[i] is used to
 * choose the index at i-th coordinate.
 */
int corrected_subspace_idx (const vox_dot center, const vox_dot dot, const vox_dot direction);

int hit_box (const struct vox_box *box, const vox_dot origin, const vox_dot direction, vox_dot res);
int hit_box_outer (const struct vox_box *box, const vox_dot origin, const vox_dot direction, vox_dot res);
int hit_plane_within_box (const vox_dot origin, const vox_dot dir, const vox_dot planedot,
                          int planenum, vox_dot res, const struct vox_box *box);
void voxel_align (vox_dot dot);
#endif

#endif
