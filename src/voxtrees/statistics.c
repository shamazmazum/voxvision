#ifdef STATISTICS
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dlfcn.h>
#include "statistics.h"
#include "tree.h"

struct statistics gstats; // Initialized with zeros

void voxtrees_clear_statistics ()
{
    memset (&gstats, 0, sizeof (gstats));
}

void voxtrees_print_statistics()
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
        "vox_ray_tree_intersection(): worst cases without intersection: %lu (%lu%%)\n"
        "vox_ray_tree_intersection(): voxels hit/skipped: %lu/%lu (%lu%% total)\n\n",
        gstats.leaf_nodes,
        gstats.inner_nodes,
        gstats.empty_nodes, (gstats.leaf_nodes) ? gstats.empty_nodes*100/gstats.leaf_nodes: 0,
        gstats.empty_volume,
        gstats.dense_dots, gstats.dense_leafs,
        gstats.rti_calls,
        gstats.rti_early_exits, (gstats.rti_calls) ? gstats.rti_early_exits*100/gstats.rti_calls: 0,
        gstats.rti_first_subspace, (gstats.rti_calls) ? gstats.rti_first_subspace*100/gstats.rti_calls: 0,
        gstats.rti_worst_cases, (gstats.rti_calls) ? gstats.rti_worst_cases*100/gstats.rti_calls: 0,
        gstats.rti_voxels_hit, gstats.rti_voxels_skipped, (gstats.rti_voxels_hit) ?
        gstats.rti_voxels_skipped*100/(gstats.rti_voxels_hit+gstats.rti_voxels_skipped): 0);

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

/*
  Poor man's backtrace. Backtrace from execinfo is very slow.
*/
static int backtr (void **addrlist, int len) __attribute__ ((noinline));
static int backtr (void **addrlist, int len)
{
    if (len == 0) return 0;
#ifdef __x86_64__
    int i = 0;
    void **bp;
    asm("\t mov %%rbp,%0" : "=r"(bp));
    while (i < len && bp != NULL)
    {
        addrlist[i] = *(bp+1);
        bp = (void**)*bp;
        i++;
    }

    return i;
#else
    return 0;
#endif
}

static int findsym (void *addr, void **sym)
{
    // Must be power of 2
#define SYMCACHESIZE 16
    static struct {
        void *addr;
        void *sym;
    } symcache[SYMCACHESIZE];
    static unsigned int symcacheptr = SYMCACHESIZE;
    static int symcachewrap = 0;

    unsigned int start = symcacheptr;
    unsigned int end = (symcachewrap) ? symcacheptr: SYMCACHESIZE;
    unsigned int idx;

    for (idx=start; idx<end; idx = (idx+1)&((1<<SYMCACHESIZE)-1))
    {
        if (symcache[idx].addr == addr)
        {
            *sym = symcache[idx].sym;
            return 1;
        }
    }

    // Item not in cache
    Dl_info info;
    if (!dladdr (addr, &info)) return 0;
    *sym = info.dli_saddr;

    unsigned int oldptr = symcacheptr;
    symcacheptr = (symcacheptr-1)&((1<<SYMCACHESIZE)-1);
    symcache[symcacheptr].addr = addr;
    symcache[symcacheptr].sym = *sym;
    if (symcacheptr > oldptr)
    {
        printf ("symcache: wrapping pointer\n");
        symcachewrap = 1;
    }

    printf ("symcache: adding entry, symcacheptr=%i\n",symcacheptr);
    return 1;
}

/*
 * CAUTION: fragile!
 * This function may be used in other recursive functions of this library
 * to execute something only once per call.
 */
int _once ()
{
    void *addrlist[3];
    int len = backtr (addrlist, 3);
    // Can do nothing, return 1
    if (len != 3) return 1;

    void *addr1, *addr2;

    // Failure, return 1
    if (!findsym (addrlist[1], &addr1)) return 1;
    if (!findsym (addrlist[2], &addr2)) return 1;

    if (addr1 != addr2) return 1;
    return 0;
}

/*
 * Used to get recursion depth (up to DEPTH_MAX). Like _once() it's fragile.
 * Returned 0 means no recursion.
 */
int _recursion_depth ()
{
    void *addrlist[DEPTH_MAX+2];
    int len = backtr (addrlist, DEPTH_MAX+2);
    if (len < 2) return 0;

    // Just return 0 on failure
    void *current_func, *addr;
    int i = 1;
    if (!findsym (addrlist[i], &current_func)) return 0;

    for (i=2; i<len; i++)
    {
        if (!findsym (addrlist[i], &addr)) return 0;
        if (addr != current_func) break;
    }

    return i-2;
}
#else
void voxtrees_print_statistics()
{
    /* Do nothing */
}

void voxtrees_clear_statistics()
{
    /* Do nothing */
}
#endif
