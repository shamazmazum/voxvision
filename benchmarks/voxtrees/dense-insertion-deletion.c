#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>
#include <gettime.h>

#define K 1000
#define L 100
#define M 100
int main ()
{
    vox_dot dot;
    int k,l,m;
    double time;
    struct vox_node *tree = NULL;

    time = gettime();
    for (k=0; k<K; k++)
    {
        for (l=0; l<L; l++)
        {
            for (m=0; m<M; m++)
            {
                dot[0] = vox_voxel[0]*k;
                dot[1] = vox_voxel[1]*l;
                dot[2] = vox_voxel[2]*m;
                vox_insert_voxel (&tree, dot);
            }
        }
    }
    time = gettime() - time;
    printf ("Insertion time %f (\"dense\" pattern). "
            "Voxels in tree %lu\n",
            time, vox_voxels_in_tree (tree));

    time = gettime();
    for (k=0; k<K; k++)
    {
        for (l=0; l<L; l++)
        {
            for (m=0; m<M; m++)
            {
                dot[0] = vox_voxel[0]*k;
                dot[1] = vox_voxel[1]*l;
                dot[2] = vox_voxel[2]*m;
                vox_delete_voxel (&tree, dot);
            }
        }
    }
    time = gettime() - time;
    printf ("Deletion time %f (\"dense\" pattern). "
            "Voxels in tree %lu\n",
            time, vox_voxels_in_tree (tree));

    vox_destroy_tree (tree); // Not needed, just in case...
    return 0;
}
