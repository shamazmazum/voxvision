/**
   @file treestat.h
   @brief Internal statistics for voxtrees library

   Functions for internal statistics printing.
**/
#ifndef _TSTATISTICS_H_
#define _TSTATISTICS_H_
#ifdef VOXTREES_SOURCE
#ifdef STATISTICS
#define FILL_RATIO_LEN 10
#include <stdint.h>
#include <sys/queue.h>

struct vox_box box;
struct statistics
{
    uint64_t leaf_nodes;
    uint64_t inner_nodes;
    uint64_t empty_nodes;

    uint64_t rti_calls;
    uint64_t rti_early_exits;
    uint64_t rti_first_subspace;
    uint64_t rti_worst_cases;
    uint64_t rti_voxels_hit;
    uint64_t rti_voxels_skipped;

    float empty_volume;
    uint64_t fill_ratio_hist[FILL_RATIO_LEN];

    uint64_t dense_leafs;
    uint64_t dense_dots;

    uint64_t dense_insertions;
    uint64_t dense_deletions;
    uint64_t leaf_insertions;
    uint64_t leaf_deletions;
};

extern struct statistics gstats;

void update_fill_ratio_hist (const struct vox_box *box, size_t n);
void update_depth_hist (int depth);

int _once ();
int _recursion_depth();

#define WITH_STAT(expr) expr
#define WITH_STAT_ONCE(expr) if (_once()) expr
#else
#define WITH_STAT(expr)
#define WITH_STAT_ONCE(expr)
#endif
#endif
/**
   \brief Print internal statistics.

   Print internal statistics (such as nodes constructed, ray-tree intersection
   calls etc.) if the library was built with statistics collection, otherwise
   do nothing.
**/
void voxtrees_print_statistics();

/**
   \brief Clear internal statistics.

   Clear internal statistics if the library was built with statistics collection,
   otherwise do nothing.
**/
void voxtrees_clear_statistics();
#endif