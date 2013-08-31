#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "tree.h"

struct tagged_coord
{
    int tag;
    float coord[N];
};

int ray_tree_intersection (struct node*, const float*, const float*, float*, int, int);
#endif
