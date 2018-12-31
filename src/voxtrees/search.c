#include <stdlib.h>
#include <math.h>

#include "geom.h"
#include "search.h"
#include "probes.h"

const struct vox_node*
vox_ray_tree_intersection (const struct vox_node *tree, const vox_dot origin,
                           const vox_dot dir, vox_dot res)
{
    vox_dot bb_inter;
    int i;
    vox_dot *plane_inter;
    int *plane_inter_idx, tmp2;
    const struct vox_node *leaf = NULL;

    /*
     * After hit_box call we can take bb_inter as a new ray origin.
     * This will help us with little optimization technik.
     */
    if (!(VOX_FULLP (tree)) ||
        !(hit_box (&(tree->bounding_box), origin, dir, bb_inter)))
    {
        WITH_STAT (VOXTREES_RTI_EARLY_EXIT());
        goto end;
    }
    /*
     * If ray hits bounding box of a dense leaf, then it hits anything inside it.
     */
    if (tree->flags & VOX_DENSE_LEAF)
    {
        leaf = tree;
        vox_dot_copy (res, bb_inter);
        WITH_STAT (VOXTREES_RTI_EARLY_EXIT());
        goto end;
    }

    if (tree->flags & VOX_LEAF)
    {
        /*
         * If passed argument is a tree leaf, do O(tree->dots_num) search for intersections
         * with voxels stored in the leaf and return closest one.
         */
        float dist_closest = INFINITY, dist_far;
        vox_dot *dots = tree->data.dots;
        struct vox_box *voxel = alloca (sizeof (struct vox_box));
        vox_dot far_inter;

        WITH_STAT (VOXTREES_RTI_VOXELS_TRAVERSED(tree->dots_num));
        for (i=0; i<tree->dots_num; i++)
        {
            vox_dot_copy (voxel->min, dots[i]);
            vox_dot_add (voxel->min, vox_voxel, voxel->max);
            if (hit_box (voxel, bb_inter, dir, far_inter))
            {
                dist_far = vox_abs_metric (bb_inter, far_inter);
                /*
                 * This is the optimization I mentioned earlier.
                 * If a distance between the node's bounding box and newly found
                 * intersection is zero, than you cannot get any closer, so return
                 * it. This works on very rare occasions in normal scenes, but helps a lot
                 * in certain conditions. The branch should be predictible.
                 */
                if (dist_far == 0)
                {
                    vox_dot_copy (res, far_inter);
                    leaf = tree;
                    WITH_STAT (VOXTREES_RTI_VOXELS_SKIPPED (tree->dots_num-i-1));
                    goto end;
                }
                if (dist_far < dist_closest)
                {
                    dist_closest = dist_far;
                    vox_dot_copy (res, far_inter);
                    leaf = tree;
                }
            }
        }
        goto end;
    }

    // not a leaf. bb_inter holds an entry point into node's bounding box
    const vox_inner_data *inner = &(tree->data.inner);
    // Find subspace index of the entry point, corrected by direction, if needed.
    int subspace = get_corrected_subspace_idx (inner->center, bb_inter, dir);

    /*
     * Look if we are lucky and the ray hits any box before it traverses the dividing
     * planes (in other words it hits a box close enough to the entry point).
     */
    if ((leaf = vox_ray_tree_intersection (inner->children[subspace], bb_inter, dir,
                                           res)))
    {
        WITH_STAT (VOXTREES_RTI_FIRST_SUBSPACE());
        goto end;
    }
    
    /*
     * No luck, search for intersections of the ray and all N axis-aligned dividing planes
     * for our N-dimentional space. If such an intersection is inside the node (it means,
     * inside its bounding box), add it to plane_inter and mark with number of plane where
     * intersection is occured.
     */
    plane_inter = alloca (sizeof (vox_dot) * VOX_N);
    plane_inter_idx = alloca (sizeof (vox_dot) * VOX_N);
    int plane_counter = 0;
    for (i=0; i<VOX_N; i++)
    {
        plane_inter_idx[plane_counter] = i;
        if (hit_plane_within_box (bb_inter, dir, inner->center, i, plane_inter[plane_counter],
                                  &(tree->bounding_box))) plane_counter++;
    }

    for (i=0; i<plane_counter; i++)
    {
        /*
          We want the closest intersection to be found,
          so find closest remaining intersection with dividing planes.
        */
        int j;
        vox_dot tmp;
        for (j=i+1; j<plane_counter; j++)
        {
            if (vox_abs_metric (bb_inter, plane_inter[j]) < vox_abs_metric (bb_inter, plane_inter[i]))
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

        /*
         * For each intersection with dividing plane call vox_ray_tree_intersection
         * recursively, using child node specified by subspace index. If an intersection
         * is found, return. Note, what we specify an entry point to that child as a new
         * ray origin.
         */
        if ((leaf = vox_ray_tree_intersection (inner->children[subspace], plane_inter[i], dir,
                                               res)))
            goto end;
    }
    WITH_STAT (VOXTREES_RTI_WORST_CASE());

end:
    return leaf;
}

int vox_tree_ball_collidep (const struct vox_node *tree, const vox_dot center, float radius)
{
    int i;
    if (!(VOX_FULLP (tree))) return 0;
    if (box_ball_interp (&(tree->bounding_box), center, radius))
    {
        if (tree->flags & VOX_DENSE_LEAF) return 1;
        if (tree->flags & VOX_LEAF)
        {
            vox_dot *dots = tree->data.dots;
            struct vox_box *voxel = alloca (sizeof (struct vox_box));
            for (i=0; i<tree->dots_num; i++)
            {
                vox_dot_copy (voxel->min, dots[i]);
                vox_dot_add (voxel->min, vox_voxel, voxel->max);
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
