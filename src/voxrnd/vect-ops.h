/**
   @file vect-ops.h
   @brief Operations with vectors and quaternions
**/
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
   \return pointer to res
**/
void vox_rotate_vector (const vox_quat, const vox_dot, vox_dot);

/**
   \brief Return multiplication of 2 quaternions
**/
void vox_quat_mul (const vox_quat, const vox_quat, vox_quat);

/**
   \brief Return dot product of 2 vectors
**/
float vox_dot_product (const vox_dot, const vox_dot);

#endif
#endif
