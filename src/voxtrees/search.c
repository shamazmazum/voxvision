#include <sys/param.h>
#include <stdlib.h>
#include <assert.h>

#include "tree.h"
#include "geom.h"
#include "search.h"

#if (defined __APPLE__ || defined __MACH__ || defined __DARWIN__ || \
     defined __FreeBSD__ || defined __DragonFly__ ||  \
     defined __OpenBSD__ || defined __NetBSD__)
#define qsort_with_arg(base,nel,width,thunk,compar) qsort_r(base,nel,width,thunk,compar)

#elif (defined _GNU_SOURCE || defined __GNU__ || defined __linux__)
#define qsort_with_arg(base,nel,width,thunk,compar) qsort_r(base, nel, width, compar, thunk)

#elif (defined _WIN32 || defined _WIN64 || defined __WINDOWS__)
#define qsort_with_arg(base,nel,width,thunk,compar) qsort_s(base,nel,width,compar,thunk)

#else
#error Cannot detect operating system
#endif

struct tagged_coord
{
    vox_uint tag;
    vox_dot coord;
};

vox_uint vox_lod = 0;

static void gen_subspaces (struct tagged_coord subspaces[], unsigned int n, vox_uint start)
{
    // Turn plane numbers into subspace indices by simply XORing
    // old subspace index with plane number
    vox_uint i;

    for (i=0; i<n; i++)
    {
        subspaces[i].tag = (1 << subspaces[i].tag) ^ start;
        start = subspaces[i].tag;
    }
}

static int compare_tagged (vox_dot origin, struct tagged_coord *c1, struct tagged_coord *c2)
{
    float *dot1 = c1->coord;
    float *dot2 = c2->coord;

    return calc_abs_metric (origin, dot1) - calc_abs_metric (origin, dot2);
}

// Maybe flollowing deserves a bit more explanation
vox_uint vox_ray_tree_intersection (struct vox_node *tree, const vox_dot origin, const vox_dot dir,
                                    vox_dot res, vox_uint depth, vox_tree_path path)
{
    vox_dot tmp;
    vox_dot inter_entry;
    vox_uint interp, i;
    struct tagged_coord plane_inter[VOX_N];

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
        int swap;
        vox_dot inter1, inter2;
        vox_dot *inter_closest = &inter1;
        vox_dot *inter_far = &inter2;
        vox_dot *inter_tmp;
        vox_leaf_data leaf = tree->data.leaf;

        for (i=0; i<leaf.dots_num; i++)
        {
            sum_vector (leaf.dots[i], vox_voxel, tmp);
            interp = hit_box (leaf.dots[i], tmp, origin, dir, *inter_far);
            if (interp)
            {
                swap = 1;
                if (found)
                {
                    float dist_closest = calc_abs_metric (origin, *inter_closest);
                    float dist_far = calc_abs_metric (origin, *inter_far);
                    if (dist_far > dist_closest) swap = 0;
                }
                if (swap)
                {
                    inter_tmp = inter_far;
                    inter_far = inter_closest;
                    inter_closest = inter_tmp;
                }
                swap = 0;
            }
            found |= interp;
        }

        if (found)
        {
            vox_dot_copy (res, *inter_closest);
            return depth;
        }
        else return 0;
    }

    // ELSE
    vox_inner_data inner = tree->data.inner;
    // Find subspace index of entry point
    vox_uint subspace_entry = get_subspace_idx (inner.center, inter_entry);
    // Look if we are lucky and the ray hits any box before it traverses the dividing planes
    // (in other workd it hits a box close enough to entry_point)
    interp = vox_ray_tree_intersection (inner.children[subspace_entry], inter_entry, dir,
                                        res, depth+1, path);
    if (interp) return interp;
    
    // No luck, search for intersections of the ray and all N axis-aligned dividing planes
    // for our N-dimentional space.
    // If such an intersection is inside the node (it means, inside its bounding box),
    // and it to plane_inter and mark with number of plane where intersection is occured.
    int plane_counter = 0;
    for (i=0; i<VOX_N; i++)
    {
        interp = hit_plane_within_box (origin, dir, inner.center, i, plane_inter[plane_counter].coord,
                                       tree->bb_min, tree->bb_max);
        plane_inter[plane_counter].tag = i;
        if (interp) plane_counter++;
    }

    // We want closest intersection to be found, so sort intersections with planes by proximity to origin
    qsort_with_arg (plane_inter, plane_counter, sizeof(struct tagged_coord), origin, compare_tagged);
    // Convert plane numbers to subspace indices
    gen_subspaces (plane_inter, plane_counter, subspace_entry);

    // For each intersection call vox_ray_tree_intersection recursively, using child node specified by subspace index
    // in tagged structure. If intersection is found, return.
    // Note, what we specify an entry point to that child as a new ray origin
    for (i=0; i<plane_counter; i++)
    {
        interp = vox_ray_tree_intersection (inner.children[plane_inter[i].tag], plane_inter[i].coord, dir,
                                            res, depth+1, path);
        if (interp) return interp;
    }

    return 0;
}

// Top call must be with depth = 1
vox_uint vox_local_rays_tree_intersection (const vox_tree_path path, const vox_dot origin, const vox_dot dir,
                                           vox_dot res, vox_uint depth, vox_uint n)
{
    if ((depth <= n) && (depth <= VOX_MAX_DEPTH_LOCAL))
    {
        vox_uint interp = vox_ray_tree_intersection (path[n-depth], origin, dir, res, 1, NULL);
        if (interp) return depth;
        else return vox_local_rays_tree_intersection (path, origin, dir, res, depth+1, n);
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
            vox_leaf_data leaf = tree->data.leaf;
            for (i=0; i<leaf.dots_num; i++)
            {
                sum_vector (leaf.dots[i], vox_voxel, tmp);
                if (box_ball_interp (leaf.dots[i], tmp, center, radius)) return 1;
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
