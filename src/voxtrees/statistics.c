#ifdef STATISTICS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <dlfcn.h>
#include "statistics.h"
#include "tree.h"

struct statistics gstats; // Initialized with zeros

STAILQ_HEAD (depth_hist_head, depth_hist_elm) depth_hist =
    STAILQ_HEAD_INITIALIZER (depth_hist);
struct depth_hist_elm {
    uint64_t number;

    STAILQ_ENTRY(depth_hist_elm) link;
};


STAILQ_HEAD (symcache_head, symcache_elm) symcache =
    STAILQ_HEAD_INITIALIZER (symcache);
struct symcache_elm {
    void *addr;
    void *sym;

    STAILQ_ENTRY(symcache_elm) link;
};

static void clear_depth_hist ()
{
    struct depth_hist_elm *elm, *next;
    elm = STAILQ_FIRST (&depth_hist);
    while (elm != NULL)
    {
        next = STAILQ_NEXT (elm, link);
        free (elm);
        elm = next;
    }
    STAILQ_INIT (&depth_hist);
}

static void stat_constructor() __attribute__((constructor));
static void stat_destructor() __attribute__((destructor));

static void print_depth_hist()
{
    struct depth_hist_elm *elm;
    int i = 0;

    printf ("Depth level /  Number of leafs\n");
    STAILQ_FOREACH (elm, &depth_hist, link)
    {
        printf ("%11i %18lu\n", i++, elm->number);
    }
}

static void print_fill_ratio_hist()
{
    int i;
    float delta = 100.0/FILL_RATIO_LEN;

    printf ("Fill ratio             / Number of leafs\n");
    for (i=0; i<FILL_RATIO_LEN; i++)
    {
        printf ("%4.1f%% <= ratio < %5.1f%% %18lu\n", i*delta, (i+1)*delta, gstats.fill_ratio_hist[i]);
    }
}

void voxtrees_clear_statistics ()
{
    memset (&gstats, 0, sizeof (gstats));
    clear_depth_hist ();
}

void update_depth_hist (int depth)
{
    int i;
    struct depth_hist_elm *elm;

    elm = STAILQ_FIRST (&depth_hist);
    for (i=0; i<=depth; i++)
    {
        if (elm == NULL)
        {
            elm = malloc (sizeof (struct depth_hist_elm));
            elm->number = 0;
            STAILQ_INSERT_TAIL (&depth_hist, elm, link);
        }
        if (i != depth)
            elm = STAILQ_NEXT(elm, link);
    }
    elm->number++;
}

void voxtrees_print_statistics()
{
    printf (
        "\x1b[35mVoxtrees library statistics\x1b[0m:\n"
        "\x1b[31mConstruction\x1b[0m:\n"
        "Leaf nodes in tree(s) constructed %lu\n"
        "Inner nodes constructed %lu\n"
        "Empty nodes constructed %lu (%lu%%)\n"
        "Empty volume in leafs %f\n"
        "%lu dots in %lu dense leaf nodes\n\n"
        "\x1b[31mSearches\x1b[0m:\n"
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

    printf ("\x1b[31mVoxtrees number-by-depth histogram\x1b[0m:\n");
    print_depth_hist();
    printf ("\n");

    printf ("\x1b[31mLeafs fill ratio histogram (without dense nodes)\x1b[0m:\n");
    print_fill_ratio_hist();
    printf ("\n");

    printf ("\x1b[31mInsertion/deletion\x1b[0m:\n"
            "Insertions in leaf nodes %lu\n"
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

#ifdef __x86_64__
#define get_bp(bp) asm("\t mov %%rbp,%0" : "=r"(bp))
#else
#define get_bp(bp) (bp) = 0
#endif

/*
  Poor man's backtrace. Backtrace from execinfo is very slow.
*/
static void* get_return_point (uintptr_t **bp)
{
    void *res;
    assert (*bp != NULL);

    res = (void*)*(*bp+1);
    *bp = (uintptr_t*)**bp;

    return res;
}

static int findsym (void *addr, void **sym)
{
    // Must be power of 2
#define SYMCACHESIZE 16
    int i = 0;
    struct symcache_elm *elm, *last;

    STAILQ_FOREACH (elm, &symcache, link)
    {
        if (elm->addr == addr)
        {
            *sym = elm->sym;
            return 1;
        }
        i++;
    }

    // Item not in the cache
    Dl_info info;
    if (!dladdr (addr, &info)) return 0;
    *sym = info.dli_saddr;

    elm = malloc (sizeof (struct symcache_elm));
    elm->addr = addr;
    elm->sym = *sym;
    STAILQ_INSERT_HEAD (&symcache, elm, link);
    if (i == SYMCACHESIZE)
    {
        printf ("symcache: removing old entry\n");
        last = STAILQ_LAST (&symcache, symcache_elm, link);
        STAILQ_REMOVE (&symcache, last, symcache_elm, link);
        free (last);
    }
    return 1;
}

/*
 * CAUTION: fragile!
 * This function may be used in other recursive functions of this library
 * to execute something only once per call.
 * Silently return 1 when in doubt.
 */
int _once ()
{
    uintptr_t *bp;
    void *addr1, *addr2, *ret1, *ret2;
    get_bp (bp);
    if (bp == NULL) return 1;

    addr1 = get_return_point (&bp);
    if (addr1 == NULL) return 1;
    addr2 = get_return_point (&bp);
    if (addr2 == NULL) return 1;

    if (!findsym (addr1, &ret1)) return 1;
    if (!findsym (addr2, &ret2)) return 1;

    if (ret1 != ret2) return 1;
    return 0;
}

/*
 * Used to get recursion depth (up to DEPTH_MAX). Like _once() it's fragile.
 * Returned 0 means no recursion.
 * In case of error silently returns trash value.
 */
int _recursion_depth ()
{
#define DEPTH_MAX 100
    uintptr_t *bp;
    int i;
    void *current_func, *addr, *func;
    get_bp (bp);

    addr = get_return_point (&bp);
    if (!findsym (addr, &current_func)) return 0;

    for (i=0; bp != NULL && i < DEPTH_MAX; i++)
    {
        addr = get_return_point (&bp);
        if (!findsym (addr, &func)) break;
        if (func != current_func) break;
    }

    return i;
}

static void stat_constructor()
{
    STAILQ_INIT (&depth_hist);
    STAILQ_INIT (&symcache);
}

static void stat_destructor()
{
    struct symcache_elm *elm, *next;
    elm = STAILQ_FIRST (&symcache);
    while (elm != NULL)
    {
        next = STAILQ_NEXT (elm, link);
        free (elm);
        elm = next;
    }
    clear_depth_hist();
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
