#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>
#include <voxrnd/simple-camera.h>
#include <gettime.h>

#define N 250
int main ()
{
    int w = 800;
    int h = 600;
    int i,sx,sy;
    double time;
    vox_dot pos = {0,0,0};
    vox_dot ray;
    struct vox_camera *camera = vox_simple_camera_iface()->construct_camera (NULL, 1.0, pos);
    camera->iface->set_window_size (camera, w, h);

    time = gettime();
    for (i=0; i<N; i++)
    {
        for (sx=0; sx<w; sx++)
        {
            for (sy=0; sy<h; sy++)
                camera->iface->screen2world (camera, ray, sx, sy);
        }
    }
    time = gettime() - time;
    printf ("<%f %f %f>\n", ray[0], ray[1], ray[2]);
    printf ("Screen->world coordinate translations took %f seconds (%i iterations)\n", time, N);

    camera->iface->destroy_camera (camera);
    return 0;
}
