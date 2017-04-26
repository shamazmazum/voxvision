/**
   @file geom.h
   @brief Data reader(s)
**/

#ifndef __READER_H_
#define __READER_H_

#include "params.h"

struct vox_node* vox_read_raw_data (const char *filename, unsigned int dim[],
                                    unsigned int samplesize, int (^test)(unsigned int sample),
                                    const char **error);

#endif
