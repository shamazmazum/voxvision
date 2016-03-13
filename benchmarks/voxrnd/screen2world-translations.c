#include <stdlib.h>
#include <stdio.h>
#include <voxtrees.h>
#include <voxrnd.h>
#include <gettime.h>

#define N 250
int main ()
{
    int i,sx,sy;
    double time;
    vox_dot pos = {0,0,0};
    vox_dot ray;
    vox_simple_camera *camera = vox_make_simple_camera (1.0, pos);
    SDL_Surface surf;
    surf.w = 800; surf.h = 600;
    struct vox_rnd_ctx *ctx = vox_make_renderer_context (&surf, NULL, &(camera->iface));

    time = gettime();
    for (i=0; i<N; i++)
    {
        for (sx=0; sx<surf.w; sx++)
        {
            for (sy=0; sy<surf.h; sy++)
                camera->iface.screen2world (camera, ray, sx, sy);
        }
    }
    time = gettime() - time;
    printf ("<%f %f %f>\n", ray[0], ray[1], ray[2]);
    printf ("Screen->world coordinate translations took %f seconds (%i iterations)\n", time, N);

    free (ctx);
    free (camera);
    return 0;
}
