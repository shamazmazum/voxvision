/**
   @file datareader.h
   @brief Data reader(s)
**/

#ifndef __READER_H_
#define __READER_H_

#include "params.h"

/**
   \brief Build a tree from raw density file.

   This type of files are common on the internet. It defines a set of voxels as
   a 3-dimensional array of dim[0] X dim[1] X dim[2] samples. These samples
   specify density of the material at that point in the space. As voxvision does
   not understand density (there can be a voxel at this point, or it can not),
   test function is introduced to determine when we must consider this sample as
   a solid voxel or not. The datafile's size must be exactly
   dim[0]*dim[1]*dim[2]*samplesize bytes.

   FIXME: The array of samples is stored on disk in row-major order.

   \param filename a name of data file.
   \param dim array dimensions.
   \param samplesize size of a sample in bytes
   \param test a block which is called with sample value as its argument. It
          must return non-zero if this voxels is considered solid or 0 otherwise.
   \param error pointer to error string.

   \return A newly created tree or NULL in case of error. In the latter case,
   error will be set to an internal static string, describing the error.
**/

struct vox_node* vox_read_raw_data (const char *filename, unsigned int dim[],
                                    unsigned int samplesize, int (^test)(unsigned int sample),
                                    const char **error);

#endif
