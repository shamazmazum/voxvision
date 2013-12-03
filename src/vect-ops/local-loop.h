#ifndef LOCALITY_HELPER_H
#define LOCALITY_HELPER_H

#include "../voxtrees/tree.h"
#include "renderer-ctx.h"

void vox_local_loop (struct vox_node*, int, void (*) (vox_rnd_context*), \
                     void (*) (vox_rnd_context*), vox_rnd_context*);

#endif
