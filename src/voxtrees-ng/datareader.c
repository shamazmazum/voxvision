#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "datareader.h"

void vox_destroy_map_3d (struct vox_map_3d *map)
{
    free (map->map);
    free (map);
}

struct vox_map_3d* vox_read_raw_data (const char *filename, unsigned int dim[3],
                                      unsigned int samplesize, int (^test)(unsigned int sample),
                                      const char **error)
{
#define BUFSIZE 4096
    unsigned char buf[BUFSIZE];
    ssize_t end = 0;
    ssize_t pos = 0;
    ssize_t read_bytes = 0;
    int i,j,k,l;
    unsigned int value;
    size_t total_size = dim[0]*dim[1]*dim[2];
    size_t map_idx = 0;
    struct vox_map_3d *map = NULL;

    int fd = open (filename, O_RDONLY);
    if (fd == -1) {*error = strerror (errno); goto done;}

    struct stat sb;
    if (fstat (fd, &sb) == -1) {*error = strerror (errno); goto done;}
    if (total_size*samplesize != sb.st_size) {*error = "Wrong size of dataset"; goto done;}
    
    map = malloc (sizeof (struct vox_map_3d));
    map->map = malloc (sizeof (int) * total_size);
    memcpy (map->dim, dim, sizeof (map->dim));
    // Initialize with empty space
    memset (map->map, 0, sizeof (int) * total_size);

    for (i=0; i<dim[0]; i++) {
        for (j=0; j<dim[1]; j++) {
            for (k=0; k<dim[2]; k++) {
                value = 0;
                for (l=0; l<samplesize; l++) {
                    if (pos==end) {
                        read_bytes = read (fd, buf, BUFSIZE);
                        if (read_bytes < 0) {*error = strerror (errno); goto done;}
                        end = read_bytes;
                        pos = 0;
                    }
                    value |= buf[pos] << (8*l);
                    pos++;
                }
                if (test (value)) map->map[map_idx] = 1;
                map_idx++;
            }
        }
    }
done:
    if (read_bytes < 0) {
        vox_destroy_map_3d (map);
        map = NULL;
    }
    if (fd != -1) close (fd);

    return map;
}
