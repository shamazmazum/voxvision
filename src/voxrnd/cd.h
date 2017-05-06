/**
   @file cd.h
   @brief Collision detection
**/
#ifndef __CD_H__
#define __CD_H__

#include "renderer.h"

struct vox_cd;

/**
   \brief Create collision detector.

   You also need to attach objects (currently, camera and context as a whole
   scene) to collision detector to make it work.
**/
struct vox_cd* vox_make_cd ();

/**
   \brief Attach a camera to collision detector.

   A camera currently has a spherical body with supplied radius.
**/
void vox_cd_attach_camera (struct vox_cd *cd, struct vox_camera *camera, float radius);

/**
   \brief Attach a renderer context to collision detector

   A whole scene contained in context will be tested for collisions.
**/
void vox_cd_attach_context (struct vox_cd *cd, struct vox_rnd_ctx *ctx);

/**
   \brief Perform collision detection.

   If any object which can move (currently, only the camera) collides with any
   another object, it will be moved to the last valid location.
**/
void vox_cd_collide (struct vox_cd *cd);

#endif
