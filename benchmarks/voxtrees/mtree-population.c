#include <stdio.h>
#include <voxtrees.h>
#include <stdlib.h>
#include <gettime.h>

#define N 5000
int main ()
{
    int i;
    double time;
    struct vox_mtree_node *tree = NULL;
    struct vox_sphere s;

    time = gettime();
    for (i=0; i<N; i++) {
        vox_dot_set (s.center, rand(), rand(), rand());
        s.radius = 1 + rand();
        vox_mtree_add_sphere (&tree, &s);
    }
    time = gettime() - time;
    printf ("Insertion took %f seconds (%i iterations)\n", time, N);

    vox_mtree_destroy (tree);
    return 0;
}
