#include <sys/param.h>
#include <stdlib.h>
#include <assert.h>

#include "tree.h"
#include "geom.h"
#include "search.h"

struct tagged_coord
{
    vox_uint tag;
    vox_dot coord;
};

vox_uint vox_lod = 0;

// Maybe flollowing deserves a bit more explanation
vox_uint vox_ray_tree_intersection (const struct vox_node *tree, const vox_dot origin, const vox_dot dir,
                                    vox_dot res, vox_uint depth, vox_tree_path path)
{
    vox_dot inter_entry;
    vox_uint interp, i;
    struct tagged_coord plane_inter[VOX_N];
    struct tagged_coord holder;

    assert (depth < VOX_MAX_DEPTH);
    if (path) path[depth-1] = tree;

    if (!(VOX_FULLP (tree))) return 0;
    if (!(hit_box (tree->bb_min, tree->bb_max, origin, dir, inter_entry))) return 0;
    if (depth == vox_lod) // Desired level of details reached -> intersection found
    {
        vox_dot_copy (res, inter_entry);
        return depth;
    }
    
    if (VOX_LEAFP(tree))
    {
        // If passed argument is a tree leaf, do O(tree->dots_num) search for intersections
        // with voxels stored in the leaf and return closest one
        int found = 0;
        vox_dot tmp;
        float dist_closest, dist_far;
        vox_dot *dots = tree->data.dots;

        // inter_entry is a "far" intersection, while res is the closest one.
        for (i=0; i<tree->dots_num; i++)
        {
            sum_vector (dots[i], vox_voxel, tmp);
            interp = hit_box (dots[i], tmp, origin, dir, inter_entry);
            if (interp)
            {
                dist_far = calc_abs_metric (origin, inter_entry);
                if (((found) && (dist_far < dist_closest)) || (!found))
                {
                    dist_closest = dist_far;
                    vox_dot_copy (res, inter_entry);
                    found = depth;
                }
            }
        }

        return found;
    }

    // ELSE
    const vox_inner_data *inner = &(tree->data.inner);
    // Find subspace index of entry point
    vox_uint subspace = get_subspace_idx (inner->center, inter_entry);
    // Look if we are lucky and the ray hits any box before it traverses the dividing planes
    // (in other workd it hits a box close enough to entry_point)
    interp = vox_ray_tree_intersection (inner->children[subspace], inter_entry, dir,
                                        res, depth+1, path);
    if (interp) return interp;
    
    // No luck, search for intersections of the ray and all N axis-aligned dividing planes
    // for our N-dimentional space.
    // If such an intersection is inside the node (it means, inside its bounding box),
    // and it to plane_inter and mark with number of plane where intersection is occured.
    int plane_counter = 0;
    for (i=0; i<VOX_N; i++)
    {
        interp = hit_plane_within_box (origin, dir, inner->center, i, plane_inter[plane_counter].coord,
                                       tree->bb_min, tree->bb_max);
        plane_inter[plane_counter].tag = i;
        if (interp) plane_counter++;
    }

    for (i=0; i<plane_counter; i++)
    {
        // We want the closest intersection to be found, so find closest remaining intersection with dividing planes
        int j;
        for (j=i+1; j<plane_counter; j++)
        {
            if (calc_abs_metric (origin, plane_inter[j].coord) < calc_abs_metric (origin, plane_inter[i].coord))
            {
                // Moving this structurfes is almost as fast as moving pointers
                holder = plane_inter[j];
                plane_inter[j] = plane_inter[i];
                plane_inter[i] = holder;
            }
        }

        // Convert a plane number into a subspace index
        subspace = subspace ^ (1 << plane_inter[i].tag);

        // For each intersection with dividing plane call vox_ray_tree_intersection recursively,
        // using child node specified by subspace index. If an intersection is found, return.
        // Note, what we specify an entry point to that child as a new ray origin
        interp = vox_ray_tree_intersection (inner->children[subspace], plane_inter[i].coord, dir,
                                            res, depth+1, path);
        if (interp) return interp;
    }

    return 0;
}

int vox_tree_ball_collidep (struct vox_node *tree, const vox_dot center, float radius)
{
    vox_uint i;
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

struct vox_search_state* vox_make_search_state (const struct vox_node *tree)
{
    struct vox_search_state *state = malloc (sizeof *state);
    state->depth = 0;
    state->local_hits = 0;
    state->tree = tree;
    return state;
}

vox_uint vox_ray_tree_intersection_wstate (struct vox_search_state *state, const vox_dot origin,
                                           const vox_dot dir, vox_dot res)
{
    vox_uint interp;
try_again:
    // KLUDGE: Bound the number of local hits in a row
    if ((state->local_hits < 4) && (state->depth) &&
        (state->depth <= state->maxdepth) && (state->depth <= VOX_MAX_DEPTH_LOCAL))
    {
        interp = vox_ray_tree_intersection (state->path[state->maxdepth-state->depth], origin, dir, res, 1, NULL);
        if (interp)
        {
            state->local_hits++;
            return 1;
        }
        else
        {
            state->depth++;
            goto try_again;
        }
    }
    state->local_hits=0;
    state->maxdepth = vox_ray_tree_intersection (state->tree, origin, dir, res, 1, state->path);
    if (state->maxdepth) state->depth = 1;
    else state->depth = 0;
    return state->depth;
}
