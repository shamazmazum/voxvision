#ifndef __READER_H_
#define __READER_H_

#include <sys/types.h>
#include <voxtrees.h>

typedef struct
{
    int x,y,z;
} dimension;

extern vox_dot mul;

int read_data (int, vox_dot**, dimension*, int, int);

#endif
