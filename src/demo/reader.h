#ifndef __READER_H_
#define __READER_H_

#include <sys/types.h>
#include <voxtrees.h>

int read_data (int fd, vox_dot **dots, int dim[], int bytes, int threshold);

#endif
