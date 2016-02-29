#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>

#include "gettime.h"

#define N 10000000
int main ()
{
    vox_dot *dots = aligned_alloc (16, sizeof(vox_dot)*N);
    int i;
    double time;
    struct vox_node *tree;

    for (i=0; i<N; i++)
    {
        // XXX: must be unique
        dots[i][0] = vox_voxel[0]*(random() & 0x0fffff);
        dots[i][1] = vox_voxel[1]*(random() & 0x0fffff);
        dots[i][2] = vox_voxel[2]*(random() & 0x0fffff);
    }

    time = gettime();
    tree = vox_make_tree (dots, N);
    time = gettime() - time;

    printf ("Static tree creation took %f seconds. "
            "Number of voxels in tree %lu\n",
            time, vox_voxels_in_tree (tree));
    vox_destroy_tree (tree);

    return 0;
}
