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
    vox_dot pos = {0,0,0};
    vox_dot angles = {1.57, 1.57, 1.57};
    struct vox_camera *camera = vox_simple_camera_interface.construct_camera (NULL, 1.0, pos);

    time = gettime();
    for (i=0; i<N; i++) camera->iface->set_rot_angles (camera, angles);
    time = gettime() - time;
    printf ("Setting rotation angles took %f seconds (%i iterations)\n", time, N);

    camera->iface->destroy_camera (camera);
    return 0;
}
