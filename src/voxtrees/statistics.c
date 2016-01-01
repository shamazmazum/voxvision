#ifdef STATISTICS
#include <stdio.h>
#include "statistics.h"
#include "params.h"

static void print_statistics() __attribute__((destructor));
struct statistics gstats; // Initialized with zeros

static void print_statistics()
{
    int i;
    gstats.leaf_nodes += gstats.empty_nodes;
    printf (
        "Voxtrees library statistics:\n\n"
        "Leaf nodes in tree(s) constructed (without dense leafs) %lu\n"
        "Empty nodes constructed %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): calls: %lu\n"
        "vox_ray_tree_intersection(): early exits: %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): ray does not cross subspaces: %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): worst cases without intersection: %lu (%lu%%)\n"
        "Empty volume in leafs %f\n"
        "%lu dots in %lu dense leaf nodes\n",
        gstats.leaf_nodes,
        gstats.empty_nodes, gstats.empty_nodes*100/gstats.leaf_nodes,
        gstats.rti_calls,
        gstats.rti_early_exits, gstats.rti_early_exits*100/gstats.rti_calls,
        gstats.rti_first_subspace,
        gstats.rti_first_subspace*100/gstats.rti_calls,
        gstats.rti_worst_cases, gstats.rti_worst_cases*100/gstats.rti_calls,
        gstats.empty_volume,
        gstats.dense_dots, gstats.dense_leafs);

    printf ("Voxtrees number-by-depth histogram:\n");
    for (i=0; i<DEPTH_MAX; i++) printf ("%lu ", gstats.depth_hist[i]);
    printf ("\n");

    printf ("Leafs fill ratio histogram:\n");
    for (i=0; i<FILL_RATIO_LEN; i++) printf ("%lu ", gstats.fill_ratio_hist[i]);
    printf ("\n");
}

void update_fill_ratio_hist (float ratio)
{
    int i;
    float inc = 1.0/FILL_RATIO_LEN;
    float r = 0;
    for (i=0; i<FILL_RATIO_LEN; i++)
    {
        r += inc;
        if (ratio <= r) gstats.fill_ratio_hist[i]++;
    }
}

float get_empty_volume (float ratio, size_t n)
{
    float vox_volume = n*vox_voxel[0]*vox_voxel[1]*vox_voxel[2];
    float bb_volume = vox_volume/ratio;
    return bb_volume - vox_volume;
}

#endif
