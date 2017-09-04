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
        "Unsuccessful searches in leaf nodes: %lu\n",
        gstats.renderer_called,
        gstats.local_unsuccessful);
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
