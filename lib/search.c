#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree.h"
#include "geom.h"
#include "search.h"

struct tagged_coord
{
    uint8_t tag;
    float coord[VOX_N];
};

unsigned int vox_lod = 0;

static void gen_subspaces (struct tagged_coord subspaces[], unsigned int n)
{
    // Turn plane numbers into subspace indices by simply XORing
    // old subspace index with plane number
    int i;

    for (i=1; i<n; i++)  subspaces[i].tag = (1 << subspaces[i].tag) ^ subspaces[i-1].tag;
}

static int compare_tagged (float *origin, struct tagged_coord *c1, struct tagged_coord *c2)
{
    float *dot1 = c1->coord;
    float *dot2 = c2->coord;

    return calc_abs_metric (origin, dot1) - calc_abs_metric (origin, dot2);
}

// Maybe flollowing deserves a bit more explanation
int vox_ray_tree_intersection (struct vox_node *tree, const float *origin, const float *dir, float *res, unsigned int depth, vox_tree_path path)
{
    float inter[VOX_N];
    float tmp[VOX_N];
    int interp, i;

    assert (depth < VOX_MAX_DEPTH);
    path[depth-1] = tree;

    if (!(VOX_FULLP (tree))) return 0;
    if (!(hit_box (tree->bb_min, tree->bb_max, origin, dir, inter))) return 0;
    if (depth == vox_lod) // Desired level of details reached -> intersection found
    {
        memcpy (res, inter, sizeof (float)*VOX_N);
        return depth;
    }
    
    if (VOX_LEAFP(tree))
    {
        // If passed argument is a tree leaf, do O(tree->dots_num) search for intersections
        // with voxels stored in the leaf and return closest one (also O(tree->dots_num))
        int count = 0;
        float intersect[VOX_MAX_DOTS][VOX_N];
        vox_leaf_data leaf = tree->data.leaf;
        for (i=0; i<leaf.dots_num; i++)
        {
            sum_vector (leaf.dots[i], vox_voxel, tmp);
            interp = hit_box (leaf.dots[i], tmp, origin, dir, inter);
            if (interp)
            {
                memcpy (intersect[count], inter, sizeof(float)*VOX_N);
                count++;
            }
        }
        if (count)
        {
            memcpy (res, closest_in_set (intersect, count, origin, calc_abs_metric), sizeof(float)*VOX_N);
            return depth;
        }
        else return 0;
    }
    // ELSE

    vox_inner_data inner = tree->data.inner;
    struct tagged_coord plane_inter[VOX_N+1];
    int plane_counter = 1;
    // Init tagged_coord structure with "entry point" in the node, so to say
    // The tag is a subspace index of entry point
    plane_inter[0].tag = get_subspace_idx (inner.center, inter);
    memcpy (plane_inter[0].coord, inter, sizeof(float)*VOX_N);

    // Now, search for intersections of the ray and all N axis-aligned planes
    // for our N-dimentional space.
    // If such an intersection is inside the node (it means, inside its bounding box),
    // and it to plane_inter and mark with number of plane where intersection is occured.
    for (i=0; i<VOX_N; i++)
    {
        interp = hit_plane (origin, dir, inner.center, i, plane_inter[plane_counter].coord);
        plane_inter[plane_counter].tag = i;
        if (interp && (dot_betweenp (tree->bb_min, tree->bb_max, plane_inter[plane_counter].coord))) plane_counter++;
    }

    // We want closest intersection to be found, so sort intersections with planes by proximity to origin
    qsort_r (plane_inter+1, plane_counter-1, sizeof(struct tagged_coord), origin, compare_tagged);
    // Convert plane numbers to subspace indices
    gen_subspaces (plane_inter, plane_counter);

    // For each intersection call vox_ray_tree_intersection recursively, using child node specified by subspace index
    // in tagged structure. If intersection is found, return.
    // Note, what we specify an entry point to that child as a new ray origin
    for (i=0; i<plane_counter; i++)
    {
        interp = vox_ray_tree_intersection (inner.children[plane_inter[i].tag], plane_inter[i].coord, dir, res, depth+1, path);
        if (interp) return interp;
    }
    
    return 0;
}

// Top call must be with idx = 0
int vox_local_rays_tree_intersection (const vox_tree_path path, const float *origin, const float *dir, float *res, unsigned int depth, unsigned int n)
{
    if ((depth <= n) && (depth <= VOX_MAX_DEPTH_LOCAL))
    {
        vox_tree_path ignored_path;
        int interp = vox_ray_tree_intersection (path[n-depth], origin, dir, res, 1, ignored_path);
        if (interp) return depth;
        else return vox_local_rays_tree_intersection (path, origin, dir, res, depth+1, n);
    }
    return 0;
}

int vox_tree_ball_collidep (struct vox_node *tree, const float *center, float radius)
{
    int i;
    if (!(VOX_FULLP (tree))) return 0;
    if (box_ball_interp (tree->bb_min, tree->bb_max, center, radius))
    {
        if (VOX_LEAFP (tree))
        {
            float tmp[VOX_N];
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
