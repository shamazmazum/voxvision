#include <assert.h>
#include <stdlib.h>
#include "search.h"
#include "geom.h"

struct dot_pair {
    vox_dot first, second;
};

static const struct vox_node*
ray_tree_intersection_leaf (const struct vox_node* tree, vox_dot starting_point,
                            const vox_dot dir, vox_dot res)
{
    vox_dot data_bb_inter;
    float precision = vox_voxel[0]/4;

    if (tree->flags & CONTAINS_HOLES) {
        if (TREE_NODATA_P (tree) ||
            !(hit_box (&(tree->data_bb), starting_point, dir, data_bb_inter)) ||
            !(vox_dot_equalp (starting_point, data_bb_inter)))
            goto return_actual_bb_inter;

        unsigned int i, n_intersections = 0;
        unsigned int dots_num = tree->leaf_data.dots_num;
        struct dot_pair *hole_intersections = alloca (sizeof (struct dot_pair) * dots_num);
        int interp;
        struct vox_box tmp;
        struct dot_pair *pair1, *pair2;

        for (i=0; i<dots_num; i++) {
            pair1 = &(hole_intersections[n_intersections]);
            vox_dot_copy (tmp.min, tree->leaf_data.dots[i]);
            vox_dot_add (tmp.min, vox_voxel, tmp.max);
            interp = hit_box (&tmp, starting_point, dir, pair1->first);
            vox_dot_copy (pair1->second, tmp.min);
            if (interp) n_intersections++;
        }
        if (n_intersections == 0) goto return_actual_bb_inter;

        // Sort intersections in order of proximity to the origin
        qsort_b (hole_intersections, n_intersections, sizeof (struct dot_pair),
                 ^(const void *d1, const void *d2) {
                     const struct dot_pair *p1 = d1;
                     const struct dot_pair *p2 = d2;
                     float mdiff = squared_metric (starting_point, p1->first) -
                                   squared_metric (starting_point, p2->first);
                     return (mdiff > 0) - (mdiff < 0);
                 });
        if (!(vox_dot_equalp (hole_intersections[0].first, data_bb_inter)))
            goto return_actual_bb_inter;

        float vox_diag = squared_diag (vox_voxel);
        for (i=0; i<n_intersections-1; i++) {
            pair1 = &(hole_intersections[i]);
            pair2 = &(hole_intersections[i+1]);
            float dist = squared_metric (pair1->first, pair2->first);
            if (dist > vox_diag) {
                vox_dot_copy (tmp.min, pair1->second);
                vox_dot_add (tmp.min, vox_voxel, tmp.max);
                interp = hit_box_outer (&tmp, starting_point, dir, res);
                assert (interp);
                return tree;
            }
        }

        // All intersections "follow" one another. Check if they pass data bb or not
        vox_dot_copy (tmp.min, hole_intersections[n_intersections-1].second);
        vox_dot_add (tmp.min, vox_voxel, tmp.max);
        interp = hit_box_outer (&tmp, starting_point, dir, res);
        assert (interp);
        vox_dot actual_bb_inter_outer;
        interp = hit_box_outer (&(tree->actual_bb), starting_point, dir, actual_bb_inter_outer);
        assert (interp);
        if (vox_dot_almost_equalp (actual_bb_inter_outer, res, precision)) return NULL;
        else return tree;
    }
    return NULL;
    // Не забудь в любом случае скопировать результат в res
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
    if (tree->flags & COVERED) {
        assert (tree->flags & CONTAINS_HOLES);
        vox_dot_copy (res, actual_bb_inter);
        return tree;
    }
    if (tree->flags & LEAF)
        return ray_tree_intersection_leaf (tree, actual_bb_inter, dir, res);

    return NULL;
}
