/**
   @file rndstat.h
   @brief Internal statistics for voxrnd library

   Functions for internal statistics printing.
**/
#ifndef _RSTATISTICS_H_
#define _RSTATISTICS_H_
#ifdef VOXRND_SOURCE
#ifdef STATISTICS
#include <stdint.h>

struct statistics
{
    uint64_t renderer_called;
    uint64_t pixels_traced;
    uint64_t leaf_mispredicts;
};

extern struct statistics gstats;

#define WITH_STAT(expr) expr
#else
#define WITH_STAT(expr)
#endif
#endif
/**
   \brief Print internal statistics.

   Print internal statistics related to the renderer if the library
   was built with statistics collection, otherwise do nothing.
**/
void voxrnd_print_statistics();

/**
   \brief Clear internal statistics.

   Clear internal statistics if the library was built with statistics
   collection, otherwise do nothing.
**/
void voxrnd_clear_statistics();
#endif
