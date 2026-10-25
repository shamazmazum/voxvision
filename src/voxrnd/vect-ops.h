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
   \return rotated vector
**/
VOX_EXPORT vox_dot vox_rotate_vector (const vox_quat base, const vox_dot vector);

/**
   \brief Return multiplication of 2 quaternions
**/
VOX_EXPORT vox_quat vox_quat_mul (const vox_quat q1, const vox_quat q2);

VOX_EXPORT vox_dot  vox_dot_normalize  (const vox_dot  dot);
VOX_EXPORT vox_quat vox_quat_normalize (const vox_quat quat);

#endif
#endif
