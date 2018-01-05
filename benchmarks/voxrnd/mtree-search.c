#include <stdio.h>
#include <voxtrees.h>
#include <voxrnd/mtree.h>
#include <stdlib.h>
#include <gettime.h>
#include <math.h>

#define N 10000
int main ()
{
    int i;
    double time;
    struct vox_mtree_node *tree = NULL;
    struct vox_sphere s;
    struct vox_sphere *spheres = vox_alloc (N * sizeof (struct vox_sphere));
    vox_dot center;
    vox_dot_set (center, 0, 0, 0);
    unsigned int seed = rand();
    srand (seed);
    __block unsigned int count = 0;

    for (i=0; i<N; i++) {
        vox_dot_set (spheres[i].center,
                     floorf (250.0 * rand() / RAND_MAX),
                     floorf (250.0 * rand() / RAND_MAX),
                     floorf (250.0 * rand() / RAND_MAX));
        spheres[i].radius = 70 + floorf (30.0 * rand() / RAND_MAX);
    }

    // Naïve search
    time = gettime();
    for (i=0; i<N; i++) {
        if (sqrtf (vox_sqr_metric (center, spheres[i].center)) < spheres[i].radius) count++;
    }
    time = gettime() - time;
    printf ("Naïve search took %f seconds (%u found in %u spheres)\n", time, count, N);
    free (spheres);

    srand (seed);
    // Populate the tree
    for (i=0; i<N; i++) {
        vox_dot_set (s.center,
                     floorf (250.0 * rand() / RAND_MAX),
                     floorf (250.0 * rand() / RAND_MAX),
                     floorf (250.0 * rand() / RAND_MAX));
        s.radius = 70 + floorf (30.0 * rand() / RAND_MAX);
        vox_mtree_add_sphere (&tree, &s);
    }

    count = 0;
    time = gettime();
    vox_mtree_spheres_containing (tree, center, ^(const struct vox_sphere *s) {
            count++;
        });
    time = gettime() - time;
    printf ("M-tree search took %f seconds (%u found in %u spheres)\n", time, count,
            vox_mtree_items (tree));

    vox_mtree_destroy (tree);
    return 0;
}
