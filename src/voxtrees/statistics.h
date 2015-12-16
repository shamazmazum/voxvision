#ifndef _STATISTICS_H_
#define _STATISTICS_H_
#ifdef STATISTICS

#define DEPTH_MAX 20

struct statistics
{
    unsigned long leaf_nodes;
    unsigned long depth_hist[DEPTH_MAX];
    unsigned long empty_nodes;

    unsigned long rti_calls;
    unsigned long rti_early_exits;
    unsigned long rti_first_subspace;
    unsigned long rti_worst_cases;
};

extern struct statistics gstats;

#endif
#endif
