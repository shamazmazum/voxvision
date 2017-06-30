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
void vox_rotate_vector (const vox_quat base, const vox_dot vector, vox_dot res);

/**
   \brief Return multiplication of 2 quaternions
**/
void vox_quat_mul (const vox_quat q1, const vox_quat q2, vox_quat res);

void vox_dot_normalize (vox_dot dot);
void vox_quat_normalize (vox_quat quat);

#endif
#endif
