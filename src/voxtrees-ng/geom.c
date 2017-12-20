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
