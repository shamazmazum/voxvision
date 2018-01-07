#include <stdio.h>
#include <voxtrees.h>
#include <gettime.h>

#define N 30000000

int main()
{
    vox_dot dot = {0, 10, 0};
    vox_dot origin = {0, 0, 0};
    vox_dot dir = {0, 1, 0};
    vox_dot inter;
    int i, j, count = 0;
    struct vox_node *tree = NULL;
    const struct vox_node *leaf;
    vox_insert_voxel (&tree, dot);

    double time = gettime();
    for (i=0; i<N; i++)
    {
        for (j=0; j<10; j++)
        {
            vox_dot_set (origin, j-5, 0, 0);
            vox_dot_set (dir, (j-5)/10.0, 1, 0);
            leaf = vox_ray_tree_intersection (tree, origin, dir, inter);
            if (leaf != NULL) count++;
        }
    }
    time = gettime () - time;

    printf ("%i ray-box intersection. %f seconds taken\n", count, time);
    return 0;
}
