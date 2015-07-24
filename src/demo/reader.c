#include <unistd.h>
#include <stdlib.h>
#include "reader.h"

vox_dot mul = {1,1,1};

int read_data (int fd, vox_dot **dots, dimension *d, int bytes, int threshold)
{
#define BUFSIZE 4096
    unsigned char buf[BUFSIZE];
    ssize_t end = 0;
    ssize_t pos = 0;

    int i,j,k,l;

    int val;
    int counter = 0;
    vox_dot *array = aligned_alloc (16, d->x*d->y*d->z*sizeof(vox_dot));
    if (array == NULL) return -1;
    
    for (i=0; i<d->x; i++)
    {
        for (j=0; j<d->y; j++)
        {
            for (k=0; k<d->z; k++)
            {
                val = 0;
                for (l=0; l<bytes; l++)
                {
                    if (pos==end)
                    {
                        ssize_t res = read (fd, buf, BUFSIZE);
                        if (res <= 0) return -1;
                        end = res;
                        pos = 0;
                    }
                    val |= buf[pos] << (8*l);
                    pos++;
                }
                if (val > threshold)
                {
                    array[counter][0] = i*mul[0];
                    array[counter][1] = j*mul[1];
                    array[counter][2] = k*mul[2];
                    counter++;
                }
            }
        }
    }
    *dots = realloc (array, sizeof(vox_dot)*counter);
    return counter;
}
