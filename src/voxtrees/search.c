#include <sys/param.h>
#include <stdlib.h>
#include <assert.h>

#include "tree.h"
#include "geom.h"
#include "search.h"

#ifdef STATISTICS
static int recursion = -1;
#endif

// Maybe following deserves a bit more explanation
int vox_ray_tree_intersection (const struct vox_node *tree, const vox_dot origin, const vox_dot dir,
                                    vox_dot res, const struct vox_node **leaf)
{
    vox_dot tmp;
    int i;
    vox_dot *plane_inter;
    int *plane_inter_idx, tmp2;
    int found = 0;

#ifdef STATISTICS
    recursion++;
    if (recursion == 0) gstats.rti_calls++;
#endif
    
    if (leaf) *leaf = tree;

    if (!(VOX_FULLP (tree)) ||
        !(hit_box (tree->bb_min, tree->bb_max, origin, dir, tmp)))
    {
#ifdef STATISTICS
        if (recursion == 0) gstats.rti_early_exits++;
#endif
        goto end;
    }

    if (VOX_LEAFP(tree))
    {
        // If passed argument is a tree leaf, do O(tree->dots_num) search for intersections
        // with voxels stored in the leaf and return closest one
        float dist_closest, dist_far;
        vox_dot *dots = tree->data.dots;

        // tmp is a "far" intersection, while res is the closest one.
        for (i=0; i<tree->dots_num; i++)
        {
            sum_vector (dots[i], vox_voxel, tmp);
            if (hit_box (dots[i], tmp, origin, dir, tmp))
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
    // Look if we are lucky and the ray hits any box before it traverses the dividing planes
    // (in other words it hits a box close enough to the entry_point)
    if (vox_ray_tree_intersection (inner->children[subspace], tmp, dir,
                                   res, leaf))
    {
        found = 1;
#ifdef STATISTICS
        if (recursion == 0) gstats.rti_first_subspace++;
#endif
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
                                  tree->bb_min, tree->bb_max)) plane_counter++;
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
#ifdef STATISTICS
    if (recursion == 0) gstats.rti_worst_cases++;
#endif

end:
#ifdef STATISTICS
    recursion--;
#endif
    return found;
}

int vox_tree_ball_collidep (struct vox_node *tree, const vox_dot center, float radius)
{
    int i;
    if (!(VOX_FULLP (tree))) return 0;
    if (box_ball_interp (tree->bb_min, tree->bb_max, center, radius))
    {
        if (VOX_LEAFP (tree))
        {
            vox_dot tmp;
            vox_dot *dots = tree->data.dots;
            for (i=0; i<tree->dots_num; i++)
            {
                sum_vector (dots[i], vox_voxel, tmp);
                if (box_ball_interp (dots[i], tmp, center, radius)) return 1;
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
