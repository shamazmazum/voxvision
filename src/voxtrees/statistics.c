#ifdef STATISTICS
#include <stdio.h>
#include "statistics.h"

static void print_statistics() __attribute__((destructor));
struct statistics gstats; // Initialized with zeros

static void print_statistics()
{
    int i;
    gstats.leaf_nodes += gstats.empty_nodes;
    printf (
        "Voxtrees library statistics:\n\n"
        "Leaf nodes in tree(s) constructed %lu\n"
        "Empty nodes constructed %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): calls: %lu\n"
        "vox_ray_tree_intersection(): early exits: %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): ray does not cross subspaces: %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): worst cases without intersection: %lu (%lu%%)\n",
        gstats.leaf_nodes,
        gstats.empty_nodes, gstats.empty_nodes*100/gstats.leaf_nodes,
        gstats.rti_calls,
        gstats.rti_early_exits, gstats.rti_early_exits*100/gstats.rti_calls,
        gstats.rti_first_subspace,
        gstats.rti_first_subspace*100/gstats.rti_calls,
        gstats.rti_worst_cases, gstats.rti_worst_cases*100/gstats.rti_calls);
    printf ("Voxtrees number-by-depth histogram:\n");
    for (i=0; i<DEPTH_MAX; i++) printf ("%lu ", gstats.depth_hist[i]);
    printf ("\n");
}

#endif
