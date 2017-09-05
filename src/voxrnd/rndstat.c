#ifdef STATISTICS
#include <string.h>
#include <stdio.h>
#include "rndstat.h"
struct statistics gstats;

void voxrnd_print_statistics()
{
    printf (
        "\x1b[35mVoxrnd library statistics\x1b[0m:\n"
        "\x1b[31mRendering\x1b[0m:\n"
        "vox_render() calls: %lu\n"
        "Pixels traced: %lu\n"
        "Leaf nodes mispredicted: %lu (%lu%%)\n",
        gstats.renderer_called,
        gstats.pixels_traced,
        gstats.leaf_mispredicts,
        (gstats.pixels_traced)? 100*gstats.leaf_mispredicts/gstats.pixels_traced: 0);
}

void voxrnd_clear_statistics()
{
    memset (&gstats, 0, sizeof (struct statistics));
}
#else
void voxrnd_print_statistics()
{
}

void voxrnd_clear_statistics()
{
}
#endif
