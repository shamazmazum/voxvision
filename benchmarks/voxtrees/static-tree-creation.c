#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>
#include <gettime.h>

#define N 10000000
int main ()
{
    vox_dot *dots = vox_alloc (sizeof(vox_dot)*N);
    int i;
    double time;
    struct vox_node *tree;
    vox_dot_set (vox_voxel, 1, 1, 1);

    for (i=0; i<N; i++)
    {
        // XXX: must be unique
        vox_dot_set (dots[i], rand(), rand(), rand());
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
