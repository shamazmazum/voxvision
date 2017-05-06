#ifndef __CD_H__
#define __CD_H__

#include "renderer.h"

struct vox_cd;

struct vox_cd* vox_make_cd ();
void vox_cd_attach_camera (struct vox_cd *cd, struct vox_camera *camera, float radius);
void vox_cd_attach_context (struct vox_cd *cd, struct vox_rnd_ctx *ctx);
void vox_cd_collide (struct vox_cd *cd);

#endif
