#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "../voxvision.h"
#include "datareader.h"
#include "map.h"

struct vox_node* vox_read_raw_data (const char *filename, unsigned int dim[3],
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
    size_t map_idx = 0;
    struct vox_map_3d *map = NULL;

    int fd = open (filename, O_RDONLY);
    if (fd == -1) {*error = strerror (errno); goto done;}

    map = vox_create_map_3d (dim);
    struct stat sb;
    if (fstat (fd, &sb) == -1) {*error = strerror (errno); goto done;}
    if (map->total_size*samplesize != sb.st_size) {*error = "Wrong size of dataset"; goto done;}

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

    struct vox_node *tree = (map != NULL)? vox_make_tree (map): NULL;
    vox_destroy_map_3d (map);

    return tree;
}

static int check_file (const char* filename)
{
    struct stat sb;

    if (!stat (filename, &sb) && S_ISREG(sb.st_mode)) return 1;
    return 0;
}

int vox_find_data_file (const char *filename, char *fullpath)
{
    // At first, try the path "as is"
    if (check_file (strncpy (fullpath, filename, MAXPATHLEN))) return 1;

    // Then try to find data file in system-wide data directory
    if (snprintf (fullpath, MAXPATHLEN,
                  "%s%s", VOX_DATA_PATH, filename) >= MAXPATHLEN)
        return 0;
    if (check_file (fullpath)) return 1;

    // Then check at ~/.voxvision
    if (snprintf (fullpath, MAXPATHLEN,
                  "%s/.voxvision/%s", getenv ("HOME"), filename) >= MAXPATHLEN)
        return 0;
    if (check_file (fullpath)) return 1;

    // Then as last resort try environment variable VOXVISION_DATA
    if (snprintf (fullpath, MAXPATHLEN,
                  "%s/%s", getenv ("VOXVISION_DATA"), filename) >= MAXPATHLEN)
        return 0;
    if (check_file (fullpath)) return 1;

    return 0;
}
