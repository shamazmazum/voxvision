#include <stdlib.h>

#include "tree.h"
#include "geom.h"
#include "search.h"

WITH_STAT (static int recursion = -1;)

// Maybe following deserves a bit more explanation
int vox_ray_tree_intersection (const struct vox_node *tree, const vox_dot origin, const vox_dot dir,
                                    vox_dot res, const struct vox_node **leaf)
{
    vox_dot tmp;
    int i;
    vox_dot *plane_inter;
    int *plane_inter_idx, tmp2;
    int found = 0;

    WITH_STAT (recursion++);
    WITH_STAT (if (recursion == 0) gstats.rti_calls++);
    
    if (leaf) *leaf = tree;

    if (!(VOX_FULLP (tree)) ||
        !(hit_box (&(tree->bounding_box), origin, dir, tmp)))
    {
        WITH_STAT (if (recursion == 0) gstats.rti_early_exits++);
        goto end;
    }
    if (VOX_DENSE_LEAFP (tree))
    {
        found = 1;
        vox_dot_copy (res, tmp);
        WITH_STAT (if (recursion == 0) gstats.rti_early_exits++);
        goto end;
    }

    if (VOX_LEAFP(tree))
    {
        // If passed argument is a tree leaf, do O(tree->dots_num) search for intersections
        // with voxels stored in the leaf and return closest one
        float dist_closest, dist_far;
        vox_dot *dots = tree->data.dots;
        struct vox_box *voxel = alloca (sizeof (struct vox_box));

        // tmp is a "far" intersection, while res is the closest one.
        for (i=0; i<tree->dots_num; i++)
        {
            vox_dot_copy (voxel->min, dots[i]);
            sum_vector (voxel->min, vox_voxel, voxel->max);
            if (hit_box (voxel, origin, dir, tmp))
            {
                dist_far = calc_abs_metric (origin, tmp);
                if (((found) && (dist_far < dist_closest)) || (!found))
                {
                    dist_closest = dist_far;
                    vox_dot_copy (res, tmp);
                    found = 1;
                }
            }
        }
        goto end;
    }

    // not a leaf. tmp holds an entry point into node's bounding box
    const vox_inner_data *inner = &(tree->data.inner);
    // Find subspace index of the entry point
    int subspace = get_subspace_idx (inner->center, tmp);
    for (i=0; i<VOX_N; i++)
    {
        /*
          If the following is true, subspace index must be fixed according to the ray's
          direction. This is because get_subspace_idx returns no useful in that special
          case.
        */
        if (inner->center[i] == origin[i])
        {
            if (dir[i] > 0) subspace &= ~(1<<i);
            else subspace |= 1<<i;
        }
    }
    // Look if we are lucky and the ray hits any box before it traverses the dividing planes
    // (in other words it hits a box close enough to the entry_point)
    if (vox_ray_tree_intersection (inner->children[subspace], tmp, dir,
                                   res, leaf))
    {
        found = 1;
        WITH_STAT (if (recursion == 0) gstats.rti_first_subspace++);
        goto end;
    }
    
    // No luck, search for intersections of the ray and all N axis-aligned dividing planes
    // for our N-dimentional space.
    // If such an intersection is inside the node (it means, inside its bounding box),
    // add it to plane_inter and mark with number of plane where intersection is occured.
    plane_inter = alloca (sizeof (vox_dot) * VOX_N);
    plane_inter_idx = alloca (sizeof (vox_dot) * VOX_N);
    int plane_counter = 0;
    for (i=0; i<VOX_N; i++)
    {
        plane_inter_idx[plane_counter] = i;
        if (hit_plane_within_box (origin, dir, inner->center, i, plane_inter[plane_counter],
                                  &(tree->bounding_box))) plane_counter++;
    }

    for (i=0; i<plane_counter; i++)
    {
        // We want the closest intersection to be found, so find closest remaining intersection with dividing planes
        int j;
        for (j=i+1; j<plane_counter; j++)
        {
            if (calc_abs_metric (origin, plane_inter[j]) < calc_abs_metric (origin, plane_inter[i]))
            {
                vox_dot_copy (tmp, plane_inter[j]);
                vox_dot_copy (plane_inter[j], plane_inter[i]);
                vox_dot_copy (plane_inter[i], tmp);
                tmp2 = plane_inter_idx[j];
                plane_inter_idx[j] = plane_inter_idx[i];
                plane_inter_idx[i] = tmp2;
            }
        }

        // Convert a plane number into a subspace index
        subspace = subspace ^ (1 << plane_inter_idx[i]);

        // For each intersection with dividing plane call vox_ray_tree_intersection recursively,
        // using child node specified by subspace index. If an intersection is found, return.
        // Note, what we specify an entry point to that child as a new ray origin
        if (vox_ray_tree_intersection (inner->children[subspace], plane_inter[i], dir,
                                       res, leaf))
        {
            found = 1;
            goto end;
        }
    }
    WITH_STAT (if (recursion == 0) gstats.rti_worst_cases++);

end:
    WITH_STAT (recursion--);
    return found;
}

int vox_tree_ball_collidep (struct vox_node *tree, const vox_dot center, float radius)
{
    int i;
    if (!(VOX_FULLP (tree))) return 0;
    if (box_ball_interp (&(tree->bounding_box), center, radius))
    {
        if (VOX_DENSE_LEAFP (tree)) return 1;
        if (VOX_LEAFP (tree))
        {
            vox_dot *dots = tree->data.dots;
            struct vox_box *voxel = alloca (sizeof (struct vox_box));
            for (i=0; i<tree->dots_num; i++)
            {
                vox_dot_copy (voxel->min, dots[i]);
                sum_vector (voxel->min, vox_voxel, voxel->max);
                if (box_ball_interp (voxel, center, radius)) return 1;
            }
        }
        else
        {
            vox_inner_data inner = tree->data.inner;
            for (i=0; i<VOX_NS; i++)
            {
                if (vox_tree_ball_collidep (inner.children[i], center, radius)) return 1;
            }
        }
    }
    return 0;
}
