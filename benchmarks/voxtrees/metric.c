#include <stdio.h>
#include <voxtrees.h>
#include <gettime.h>

#define N 300000000

int main()
{
    vox_dot dot1, dot2;
    float acc = 0;
    double time;
    int i;

    time = gettime();
    for (i=0; i<N; i++)
    {
        vox_dot_set (dot1, i, i+1, i+2);
        vox_dot_set (dot2, i+1, i+2, i+3);
        acc += vox_sqr_metric (dot1, dot2);
    }
    time = gettime () - time;
    printf ("vox_sqr_metric = %f, time = %f\n", acc, time);

    acc = 0;
    time = gettime();
    for (i=0; i<N; i++)
    {
        vox_dot_set (dot1, i, i+1, i+2);
        vox_dot_set (dot2, i+1, i+2, i+3);
        acc += vox_abs_metric (dot1, dot2);
    }
    time = gettime () - time;
    printf ("vox_abs_metric = %f, time = %f\n", acc, time);
    printf ("Metric functions were called %i times\n", N);

    return 0;
}
