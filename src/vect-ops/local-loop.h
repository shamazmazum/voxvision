#ifndef LOCALITY_HELPER_H
#define LOCALITY_HELPER_H

#include "../voxtrees/tree.h"

typedef struct
{
    vox_dot origin;
    vox_dot dir;
    vox_dot inter;
    void *user_data;
} vox_rnd_context;

void vox_local_loop (struct vox_node*, int, void (*) (vox_rnd_context*), \
                     void (*) (vox_rnd_context*), vox_rnd_context*);

#endif
