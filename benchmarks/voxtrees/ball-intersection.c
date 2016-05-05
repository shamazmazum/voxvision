#include <stdio.h>
#include <voxtrees.h>
#include <gettime.h>

#define N 1000000

int main ()
{
    vox_dot dot = {0,0,0};
    vox_dot center;
    int i,j,counter = 0;

    struct vox_node *tree = NULL;
    vox_insert_voxel (&tree, dot);

    double time = gettime();
    for (i=0; i<N; i++)
    {
        for (j=-100; j<100; j++)
        {
#ifdef SSE_INTRIN
            _mm_store_ps (center, _mm_set_ps (0, 0, 10, j));
#else
            center[0] = 0;
            center[1] = 10;
            center[2] = j;
#endif
            counter += vox_tree_ball_collidep (tree, center, 1000);
        }
    }
    time = gettime() - time;

    printf ("%i iterations, %f seconds taken\n", counter, time);
    vox_destroy_tree (tree);
    return 0;
}
