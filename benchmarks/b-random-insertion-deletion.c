#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>

#include "gettime.h"

#define N 10000000
int main ()
{
    vox_dot dot;
    int i;
    double time;
    struct vox_node *tree = NULL;

    srandom (123);
    time = gettime();
    for (i=0; i<N; i++)
    {
        dot[0] = vox_voxel[0]*(random() & 0x0fffff);
        dot[1] = vox_voxel[1]*(random() & 0x0fffff);
        dot[2] = vox_voxel[2]*(random() & 0x0fffff);
        vox_insert_voxel (&tree, dot);
    }
    time = gettime() - time;
    printf ("Insertion time %f (\"random\" pattern). "
            "Voxels in tree %lu\n",
            time, vox_voxels_in_tree (tree));

    srandom (123);
    time = gettime();
    for (i=0; i<N; i++)
    {
        dot[0] = vox_voxel[0]*(random() & 0x0fffff);
        dot[1] = vox_voxel[1]*(random() & 0x0fffff);
        dot[2] = vox_voxel[2]*(random() & 0x0fffff);
        vox_delete_voxel (&tree, dot);
    }
    time = gettime() - time;
    printf ("Deletion time %f (\"random\" pattern). "
            "Voxels in tree %lu\n",
            time, vox_voxels_in_tree (tree));

    vox_destroy_tree (tree); // Not needed, just in case...
    return 0;
}
