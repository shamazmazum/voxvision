#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>
#include <voxrnd/simple-camera.h>
#include <gettime.h>

#define N 10000000
int main ()
{
    int i;
    double time;
    vox_dot angles = {0.1, 0.2, 0.3};
    struct vox_camera *camera = vox_simple_camera_iface()->construct_camera (NULL);

    time = gettime();
    for (i=0; i<N; i++) camera->iface->rotate_camera (camera, angles);
    time = gettime() - time;
    printf ("Rotating camera took %f seconds (%i iterations)\n", time, N);

    camera->iface->destroy_camera (camera);
    return 0;
}
