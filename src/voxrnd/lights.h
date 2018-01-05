#ifndef __LIGHTS_H__
#define __LIGHTS_H__
#include "renderer.h"

int vox_insert_point_light (struct vox_rnd_ctx *context,
                            const struct vox_sphere *position,
                            Uint32 color);

int vox_delete_point_light (struct vox_rnd_ctx *context,
                            const struct vox_sphere *position);


#endif
