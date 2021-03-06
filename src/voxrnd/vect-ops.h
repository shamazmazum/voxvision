#ifndef __VECT_OPS__
#define __VECT_OPS__

#include "../voxvision.h"

#ifdef VOXRND_SOURCE
/**
   \brief Rotate one vector around another

   Quaternion which specifies the rotation must be in form \f$q =
   \cos \phi + p \sin \phi\f$, where \f$2 \phi\f$ is an angle of
   rotation and \f$p\f$, \f$\vert p \vert = 1\f$ is a vector,
   around which rotation is performed.

   \param base rotation quaternion \f$q\f$
   \param vector a vector to rotate
   \param res a vector to store the result
**/
VOX_EXPORT void vox_rotate_vector (const vox_quat base, const vox_dot vector, vox_dot res);

/*
 * Same as `vox_rotate_vector()`, but expects and returns vector coordinates as
 * in quaternion data layout.
 */
VOX_EXPORT void vox_rotate_vector_q (const vox_quat base, const vox_quat vector, vox_quat res);

/**
   \brief Return multiplication of 2 quaternions
**/
VOX_EXPORT void vox_quat_mul (const vox_quat q1, const vox_quat q2, vox_quat res);

VOX_EXPORT void vox_dot_normalize (vox_dot dot);
VOX_EXPORT void vox_quat_normalize (vox_quat quat);

VOX_EXPORT void vox_quat_set_identity (vox_quat quat);

#endif
#endif
