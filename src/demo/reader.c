#include <unistd.h>
#include <stdlib.h>
#include "reader.h"

int read_data (int fd, vox_dot **dots, int dim[], int bytes, int threshold)
{
#define BUFSIZE 4096
    unsigned char buf[BUFSIZE];
    ssize_t end = 0;
    ssize_t pos = 0;

    int i,j,k,l;

    int val;
    int counter = 0;
    vox_dot *array = aligned_alloc (16, dim[0]*dim[1]*dim[2]*sizeof(vox_dot));
    if (array == NULL) return -1;
    
    for (i=0; i<dim[0]; i++)
    {
        for (j=0; j<dim[1]; j++)
        {
            for (k=0; k<dim[2]; k++)
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
                    array[counter][0] = i*vox_voxel[0];
                    array[counter][1] = j*vox_voxel[1];
                    array[counter][2] = k*vox_voxel[2];
                    counter++;
                }
            }
        }
    }
    *dots = realloc (array, sizeof(vox_dot)*counter);
    return counter;
}
