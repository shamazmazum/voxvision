#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>
#include <voxrnd.h>
#include <gettime.h>

#define N 10000000
int main ()
{
    int i;
    double time;
    vox_dot pos = {0,0,0};
    vox_dot angles = {1.57, 1.57, 1.57};
    vox_simple_camera *camera = vox_make_simple_camera (1.0, pos);

    time = gettime();
    for (i=0; i<N; i++) camera->iface->set_rot_angles (camera, angles);
    time = gettime() - time;
    printf ("Setting rotation angles took %f seconds (%i iterations)\n", time, N);

    free (camera);
    return 0;
}
