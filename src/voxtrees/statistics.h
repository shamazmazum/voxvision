#ifndef _STATISTICS_H_
#define _STATISTICS_H_
#ifdef STATISTICS
#define WITH_STAT(expr) expr
#define DEPTH_MAX 20
#define FILL_RATIO_LEN 10

struct statistics
{
    unsigned long leaf_nodes;
    unsigned long depth_hist[DEPTH_MAX];
    unsigned long empty_nodes;

    unsigned long rti_calls;
    unsigned long rti_early_exits;
    unsigned long rti_first_subspace;
    unsigned long rti_worst_cases;

    float empty_volume;
    unsigned long fill_ratio_hist[FILL_RATIO_LEN];

    unsigned long dense_leafs;
    unsigned long dense_dots;
};

extern struct statistics gstats;

void update_fill_ratio_hist (float ratio);
float get_empty_volume (float ratio, size_t n);

#else
#define WITH_STAT(expr) 
#endif
#endif
