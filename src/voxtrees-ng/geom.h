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

int dot_inside_box (const struct vox_box *box, const vox_dot dot, int strong);
int box_inside_box (const struct vox_box *outer, const struct vox_box *inner, int strong);
int voxel_inside_box (const struct vox_box *box, const vox_dot voxel, int strong);

void subspace_box (const struct vox_box *space, const vox_dot center,
                   struct vox_box *subspace, int subspace_idx);
int subspace_idx (const vox_dot center, const vox_dot dot);

float squared_metric (const vox_dot d1, const vox_dot d2);
int hit_box (const struct vox_box *box, const vox_dot origin, const vox_dot direction, vox_dot res);
int hit_box_outer (const struct vox_box *box, const vox_dot origin, const vox_dot direction, vox_dot res);
float squared_diag (const vox_dot dot);
void voxel_align (vox_dot dot);
int vox_dot_almost_equalp (const vox_dot d1, const vox_dot d2, float prec);
#endif
