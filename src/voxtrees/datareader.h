/**
   @file datareader.h
   @brief Data reader(s)
**/

#ifndef __READER_H_
#define __READER_H_

#include <stdint.h>
#include "params.h"

/**
   \brief Build a tree from a density file.

   Currently only Numpy .npy format is supported. Array element type
   (dtype) must be one of 'bool', 'uint8', 'uint16' or 'uint32'. The
   array must be in row-major format.

   \param filename a name of data file.
   \param test a block which is called with sample value as its argument. It
          must return non-zero if this voxels is considered solid or 0 otherwise.
   \param error pointer to an error string or `NULL`.

   \return A newly created tree or NULL in case of error. In the latter case,
           error will be set to an internal static string, describing the error.
**/

VOX_EXPORT struct vox_node*
vox_read_data (const char *filename,
               bool (^test)(uint32_t sample, uint8_t bps),
               const char **error);

#endif
