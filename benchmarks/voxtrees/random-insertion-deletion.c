#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>
#include <gettime.h>

#define N 10000000
int main ()
{
    vox_dot dot;
    int i;
    double time;
    struct vox_node *tree = NULL;

    vox_dot_set (vox_voxel, 1, 1, 1);
    srand (123);
    time = gettime();
    for (i=0; i<N; i++)
    {
        vox_dot_set (dot, rand(), rand(), rand());
        vox_insert_voxel (&tree, dot);
    }
    time = gettime() - time;
    printf ("Insertion time %f (\"random\" pattern). "
            "Voxels in tree %lu\n",
            time, vox_voxels_in_tree (tree));

    srand (123);
    time = gettime();
    for (i=0; i<N; i++)
    {
        vox_dot_set (dot, rand(), rand(), rand());
        vox_delete_voxel (&tree, dot);
    }
    time = gettime() - time;
    printf ("Deletion time %f (\"random\" pattern). "
            "Voxels in tree %lu\n",
            time, vox_voxels_in_tree (tree));

    vox_destroy_tree (tree); // Not needed, just in case...
    return 0;
}
