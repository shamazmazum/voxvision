#include <sys/time.h>
#include <stdlib.h>

double gettime ()
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (double)tv.tv_sec + (0.000001 * (double)tv.tv_usec);
}
