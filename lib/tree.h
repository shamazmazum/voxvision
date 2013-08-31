#ifndef _TREE_H_
#define _TREE_H_

#include <stdint.h>
#include "params.h"

#define LEAFP(node) !((node)->children)

struct node
{
    float bb_min[N];
    float bb_max[N];

    struct node **children;
    
    uint8_t dots_num;
    float dots[MAX_DOTS][N];
};

void sum_vector (const float*, const float*, float*);
uint8_t get_subspace_idx (const float*, const float*);
struct node* make_tree (float [][N], int);
int voxels_in_tree (struct node*);
int inacc_depth (struct node*, int);
float inacc_balanceness (struct node*);
void destroy_tree (struct node*);

#endif
