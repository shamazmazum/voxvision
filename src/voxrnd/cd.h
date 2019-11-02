/**
   @file cd.h
   @brief Collision detection
**/
#ifndef __CD_H__
#define __CD_H__

#include "renderer.h"

/**
   @struct vox_cd
   \brief Opaque structure for collision detector.

   User cannot do anything with it directly and must use `vox_cd_*` functions
   instead.
**/
struct vox_cd;

/**
   \brief Create collision detector.

   You also need to attach objects (currently, camera and context as a whole
   scene) to collision detector to make it work.
**/
VOX_EXPORT struct vox_cd* vox_make_cd ();

/**
   \brief Attach a camera to collision detector.

   A camera currently has a spherical body with supplied radius.
**/
VOX_EXPORT void vox_cd_attach_camera (struct vox_cd *cd, struct vox_camera *camera, float radius);

/**
   \brief Attach a renderer context to collision detector

   A whole scene contained in context will be tested for collisions.
**/
VOX_EXPORT void vox_cd_attach_context (struct vox_cd *cd, struct vox_rnd_ctx *ctx);

/**
   \brief Perform collision detection.

   If any object which can move (currently, only the camera) collides with any
   another object, it will be moved to the last valid location.
**/
VOX_EXPORT void vox_cd_collide (struct vox_cd *cd);

/**
   \brief Add gravity to the world

   \param cd a collision detection instance
   \param gravity a gravity vector.

   Each time `vox_cd_collide()` is called, all movable objects will be moved
   towards direction specified by gravity vector until they hit a hard ground.

   Gravity vector of {0,0,0} turns off gravity.
**/
VOX_EXPORT void vox_cd_gravity (struct vox_cd *cd, const vox_dot gravity);

#endif
