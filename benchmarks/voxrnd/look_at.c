#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>
#include <voxrnd/camera.h>
#include <gettime.h>

#define N 10000000
int main ()
{
    int i;
    double time;
    vox_dot coord = {0, 100, 20};
    struct vox_camera *camera = vox_camera_methods ("simple-camera")->construct_camera (NULL);

    time = gettime();
    for (i=0; i<N; i++) camera->iface->rotate_camera (camera, coord);
    time = gettime() - time;
    printf ("Looking at something took %f seconds (%i iterations)\n", time, N);

    camera->iface->destroy_camera (camera);
    return 0;
}
