#ifdef STATISTICS
#include <stdio.h>
#include <assert.h>
#include "statistics.h"
#include "tree.h"

static void print_statistics() __attribute__((destructor));
struct statistics gstats; // Initialized with zeros

static void print_statistics()
{
    int i;
    printf (
        "Voxtrees library statistics:\n"
        "Leaf nodes in tree(s) constructed %lu\n"
        "Inner nodes constructed %lu\n"
        "Empty nodes constructed %lu (%lu%%)\n"
        "Empty volume in leafs %f\n"
        "%lu dots in %lu dense leaf nodes\n\n"
        "vox_ray_tree_intersection(): calls: %lu\n"
        "vox_ray_tree_intersection(): early exits: %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): ray does not cross subspaces: %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): worst cases without intersection: %lu (%lu%%)\n\n",
        gstats.leaf_nodes,
        gstats.inner_nodes,
        gstats.empty_nodes, gstats.empty_nodes*100/gstats.leaf_nodes,
        gstats.empty_volume,
        gstats.dense_dots, gstats.dense_leafs,
        gstats.rti_calls,
        gstats.rti_early_exits, (gstats.rti_calls) ? gstats.rti_early_exits*100/gstats.rti_calls: 0,
        gstats.rti_first_subspace, (gstats.rti_calls) ? gstats.rti_first_subspace*100/gstats.rti_calls: 0,
        gstats.rti_worst_cases, (gstats.rti_calls) ? gstats.rti_worst_cases*100/gstats.rti_calls: 0);

    printf ("Voxtrees number-by-depth histogram:\n");
    for (i=0; i<DEPTH_MAX; i++) printf ("%lu ", gstats.depth_hist[i]);
    printf ("\n");

    printf ("Leafs fill ratio histogram (without dense nodes):\n");
    for (i=0; i<FILL_RATIO_LEN; i++) printf ("%lu ", gstats.fill_ratio_hist[i]);
    printf ("\n\n");

    printf ("Insertions in leaf nodes %lu\n"
            "Deletions from leaf nodes %lu\n"
            "Insertions in dense leaf nodes %lu\n"
            "Deletions from dense leaf nodes %lu\n",
            gstats.leaf_insertions,
            gstats.leaf_deletions,
            gstats.dense_insertions,
            gstats.dense_deletions);
}

static float fill_ratio (const struct vox_box *box, size_t n)
{
    float bb_volume, vox_volume;
    vox_dot size;
    int i;

    for (i=0; i<VOX_N; i++) size[i] = box->max[i] - box->min[i];
    bb_volume = size[0]*size[1]*size[2];
    vox_volume = vox_voxel[0]*vox_voxel[1]*vox_voxel[2];
    vox_volume *= n;
    gstats.empty_volume += bb_volume - vox_volume;
    return vox_volume/bb_volume;
}

void update_fill_ratio_hist (const struct vox_box *box, size_t n)
{
    int idx;
    float ratio;

    ratio = fill_ratio (box, n);
    idx = (int)(ratio*FILL_RATIO_LEN);
    assert (idx < FILL_RATIO_LEN);
    gstats.fill_ratio_hist[idx]++;
}

#endif
