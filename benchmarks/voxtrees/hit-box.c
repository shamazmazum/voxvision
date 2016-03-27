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
    int i, j, interp = 0;
    struct vox_node *tree = NULL;
    vox_insert_voxel (&tree, dot);

    double time = gettime();
    for (i=0; i<N; i++)
    {
        for (j=0; j<10; j++)
        {
            origin[0] = j-5;
            dir[0] = origin[0]/10;
            interp += vox_ray_tree_intersection (tree, origin, dir, inter, NULL);
        }
    }
    time = gettime () - time;

    printf ("%i ray-box intersection. %f seconds taken\n", interp, time);
    return 0;
}
