#ifndef DATAREADER_H
#define DATAREADER_H

#include "tree.h"

struct vox_node* vox_read_raw_data (const char *filename, unsigned int dim[],
                                    unsigned int samplesize, int (^test)(unsigned int sample),
                                    const char **error);
int vox_find_data_file (const char *filename, char *fullpath);

#endif
