#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include "tree.h"
#include "datareader.h"

struct vox_node* vox_read_raw_data (const char *filename, unsigned int dim[],
                                    unsigned int samplesize, int (^test)(unsigned int sample),
                                    const char **error)
{
#define SETERROR(x) if (error != NULL) *error = (x)
#define SETSYSERROR() if (error != NULL) *error = strerror(errno)

    struct vox_node *tree = NULL;
    int fd = open (filename, O_RDONLY);
    if (fd == -1) {
        SETSYSERROR();
        goto done;
    }
    vox_dot *array;

#define BUFSIZE 4096
    unsigned char buf[BUFSIZE];
    ssize_t end = 0;
    ssize_t pos = 0;

    unsigned int i,j,k,l,n = dim[0]*dim[1]*dim[2];
    size_t counter = 0;
    unsigned int value;

    struct stat sb;
    if (fstat (fd, &sb) == -1) {
        SETSYSERROR();
        goto closefd;
    }
    if (n*samplesize != sb.st_size) {
        SETERROR ("Wrong size of dataset");
        goto closefd;
    }

    array = vox_alloc (n*sizeof(vox_dot));
    if (array == NULL) {
        SETSYSERROR();
        goto freemem;
    }

    for (i=0; i<dim[0]; i++) {
        for (j=0; j<dim[1]; j++) {
            for (k=0; k<dim[2]; k++) {
                value = 0;
                for (l=0; l<samplesize; l++) {
                    if (pos==end) {
                        ssize_t res = read (fd, buf, BUFSIZE);
                        if (res <= 0) {
                            SETSYSERROR();
                            goto freemem;
                        }
                        end = res;
                        pos = 0;
                    }
                    value |= buf[pos] << (8*l);
                    pos++;
                }
                if (test (value)) {
                    array[counter][0] = i*vox_voxel[0];
                    array[counter][1] = j*vox_voxel[1];
                    array[counter][2] = k*vox_voxel[2];
                    counter++;
                }
            }
        }
    }
    tree = vox_make_tree (array, counter);

freemem:
    free (array);
closefd:
    close (fd);
done:
    return tree;
}

static int check_file (const char* filename)
{
    struct stat sb;

    if (!stat (filename, &sb) && S_ISREG(sb.st_mode)) return 1;
    return 0;
}

 __attribute__((constructor))
static void set_voxvision_data ()
{
    char path[MAXPATHLEN];
    if (snprintf (path, MAXPATHLEN, "%s/.voxvision", getenv ("HOME")) >= MAXPATHLEN)
        return;

    setenv ("VOXVISION_DATA", path, 0);
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

    // Check if VOXVISION_DATA environment variable contains directory with needed file.
    if (getenv ("VOXVISION_DATA") != NULL) {
        if (snprintf (fullpath, MAXPATHLEN,
                      "%s/%s", getenv ("VOXVISION_DATA"), filename) >= MAXPATHLEN)
            return 0;
        if (check_file (fullpath)) return 1;
    }

    return 0;
}
