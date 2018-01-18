#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "search.h"
#include "geom.h"
#include "probes.h"

struct dot_pair {
    vox_dot first, second;
};

/*
 * In this function we return the closest possible intersection with voxels in
 * the leaf which contains solid voxels.
 */
static const struct vox_node*
ray_tree_intersection_leaf_solid (const struct vox_node* tree, vox_dot starting_point,
                                  const vox_dot dir, vox_dot res)
{
    WITH_STAT (VOXTREES_NG_TRAVERSE_LEAF_SOLID ());
#if 0
    /* Do we set NODATA for "solid" leafs? */
    if (TREE_NODATA_P (tree)) return NULL;
#endif
    float dist, dist_closest = INFINITY;
    struct vox_box tmp;
    vox_dot far_inter;
    int i;

    for (i=0; i<tree->leaf_data.dots_num; i++) {
        vox_dot_copy (tmp.min, tree->leaf_data.dots[i]);
        vox_dot_add (tmp.min, vox_voxel, tmp.max);
        if (hit_box (&tmp, starting_point, dir, far_inter)) {
            dist = vox_abs_metric (starting_point, far_inter);
            if (dist == 0) {
                vox_dot_copy (res, far_inter);
                return tree;
            }
            if (dist < dist_closest) {
                dist_closest = dist;
                vox_dot_copy (res, far_inter);
            }
        }
    }

    if (dist_closest != INFINITY) return tree;

    return NULL;
}

/*
 * This function is much trickier than for solid voxels. It finds the closest
 * hit with voxel NOT belonging the leaf.
 */
static const struct vox_node*
ray_tree_intersection_leaf_hole (const struct vox_node* tree, vox_dot starting_point,
                                 const vox_dot dir, vox_dot res)
{
    WITH_STAT (VOXTREES_NG_TRAVERSE_LEAF_HOLE ());
    /*
     * If we do not hit data bounding box, return intersection with actual
     * bounding box.
     */
    if (TREE_NODATA_P (tree) ||
        !(dot_inside_box (&(tree->data_bb), starting_point, 0))) {
        WITH_STAT (VOXTREES_NG_DATA_BB_INSIDE_ACTUAL_BB());
        goto return_actual_bb_inter;
    }

    unsigned int i, n_intersections = 0;
    unsigned int dots_num = tree->leaf_data.dots_num;
    struct dot_pair *hole_intersections = alloca (sizeof (struct dot_pair) * dots_num);
    struct vox_box tmp;
    struct dot_pair *pair1, *pair2;

    for (i=0; i<dots_num; i++) {
        pair1 = &(hole_intersections[n_intersections]);
        vox_dot_copy (tmp.min, tree->leaf_data.dots[i]);
        vox_dot_add (tmp.min, vox_voxel, tmp.max);
        if (hit_box (&tmp, starting_point, dir, pair1->first)) {
            vox_dot_copy (pair1->second, tmp.min);
            n_intersections++;
        }
    }
    if (n_intersections == 0) goto return_actual_bb_inter;

    /* Sort all intersections with holes in order of proximity to the origin. */
    qsort_b (hole_intersections, n_intersections, sizeof (struct dot_pair),
             ^(const void *d1, const void *d2) {
                 const struct dot_pair *p1 = d1;
                 const struct dot_pair *p2 = d2;
                 float mdiff = vox_abs_metric (starting_point, p1->first) -
                     vox_abs_metric (starting_point, p2->first);
                 return (mdiff > 0) - (mdiff < 0);
             });

    /*
     * Check the first intersection. Maybe it is not even lying on data bounding
     * box (i.e. it's inside it). In this case, return.
     */
    if (dot_inside_box (&(tree->data_bb), hole_intersections[0].first, 1))
        goto return_actual_bb_inter;

    /* Check all intersections for connectivity one with another. */
    for (i=0; i<n_intersections-1; i++) {
        pair1 = &(hole_intersections[i]);
        pair2 = &(hole_intersections[i+1]);

        vox_dot_copy (tmp.min, pair1->second);
        vox_dot_add (tmp.min, vox_voxel, tmp.max);

        if (!dot_inside_box (&tmp, pair2->first, 0)) {
            /*
             * Two not connected intersections found. Return intersection with
             * the first "from the other side".
             */
            if (!hit_box_outer (&tmp, starting_point, dir, res)) {
                /*
                 * Here and below: if hit_box_outer() does not return 1, this is
                 * computational error, but it's not so serious to trigger
                 * abort(). Investigate later.
                 */
                return NULL;
            }
            return tree;
        }
    }

    vox_dot_copy (tmp.min, hole_intersections[n_intersections-1].second);
    vox_dot_add (tmp.min, vox_voxel, tmp.max);

    /*
     * Here the ray passes all the way through data bounding box. Last chance
     * for it to hit anything is to hit space belonging to actual bounding box
     * past data bounding box. Check this possibility.
     */
    if (!hit_box_outer (&tmp, starting_point, dir, res)) return NULL;

    if (dot_inside_box (&(tree->actual_bb), res, 1)) return tree;
    else return NULL;

return_actual_bb_inter:
    vox_dot_copy (res, starting_point);
    return tree;
}

const struct vox_node*
vox_ray_tree_intersection (const struct vox_node* tree, const vox_dot origin,
                           const vox_dot dir, vox_dot res)
{
    vox_dot actual_bb_inter;

    if (tree == NULL) return NULL;
    if (!(hit_box (&(tree->actual_bb), origin, dir, actual_bb_inter))) return NULL;
    /*
     * Covered nodes do not require any extra check because their data bounding
     * box is completely inside the actual bounding box.
     */
    if (tree->flags & COVERED) {
        WITH_STAT (VOXTREES_NG_TRAVERSE_NODE_COVERED ());
        assert (tree->flags & CONTAINS_HOLES);
        vox_dot_copy (res, actual_bb_inter);
        return tree;
    }

    if (tree->flags & LEAF) {
        if (tree->flags & CONTAINS_HOLES)
            return ray_tree_intersection_leaf_hole (tree, actual_bb_inter, dir, res);
        else
            return ray_tree_intersection_leaf_solid (tree, actual_bb_inter, dir, res);
    }

    /*
     * For nodes which contain holes, check is data bounding box shares no
     * common points with edge of the actual bounding box.
     */
    if ((tree->flags & CONTAINS_HOLES) &&
        !(dot_inside_box (&(tree->data_bb), actual_bb_inter, 0))) {
        vox_dot_copy (res, actual_bb_inter);
        return tree;
    }

    const struct vox_node *leaf = NULL;
    const struct inner_node *inner = &(tree->inner_data);
    int i;

    /*
     * Find the subspace index of the entry point. Note, that if we are here,
     * intersection with actual bounding box == intersection with data bounding
     * box on the ray's path.
     */
    int subspace = corrected_subspace_idx (inner->center, actual_bb_inter, dir);

    /*
     * Look if we are lucky and the ray hits any box before it traverses the dividing
     * planes (in other words it hits a box close enough to the entry point).
     */
    if ((leaf = vox_ray_tree_intersection (inner->children[subspace], actual_bb_inter, dir,
                                           res)) != NULL) {
        return leaf;
    }
    
    /*
     * No luck, search for intersections of the ray and all N axis-aligned dividing planes
     * for our N-dimentional space. If such an intersection is inside the node (it means,
     * inside its bounding box), add it to plane_inter and mark with number of plane where
     * intersection is occured.
     */
    vox_dot *plane_inter = alloca (sizeof (vox_dot) * 3);
    int *plane_inter_idx = alloca (sizeof (vox_dot) * 3);
    int plane_counter = 0;
    for (i=0; i<3; i++)
    {
        plane_inter_idx[plane_counter] = i;
        if (hit_plane_within_box (actual_bb_inter, dir, inner->center, i,
                                  plane_inter[plane_counter], &(tree->data_bb)))
            plane_counter++;
    }

    for (i=0; i<plane_counter; i++)
    {
        /*
          We want the closest intersection to be found,
          so find closest remaining intersection with dividing planes.
        */
        int j;
        vox_dot tmp;
        int tmp2;
        for (j=i+1; j<plane_counter; j++)
        {
            if (vox_abs_metric (actual_bb_inter, plane_inter[j]) <
                vox_abs_metric (actual_bb_inter, plane_inter[i]))
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
                                               res)) != NULL) {
            return leaf;
        }
    }

    /*
     * For nodes which contain holes, we still can hit "the other side" of the
     * actual bounding box, check this possibility.
     */
    if (tree->flags & CONTAINS_HOLES) {
        if (!hit_box_outer (&(tree->data_bb), origin, dir, res)) return NULL;
        if (dot_inside_box (&(tree->actual_bb), res, 1)) return tree;
        else return NULL;
    }

    return NULL;
}
