#ifndef __READER_H_
#define __READER_H_

#include "params.h"

struct vox_node* vox_read_raw_data (const char *filename, unsigned int dim[],
                                    unsigned int samplesize, unsigned int threshold,
                                    const char **error);

#endif
