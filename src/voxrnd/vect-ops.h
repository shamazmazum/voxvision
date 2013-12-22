/**
   @file vect-ops.h
   @brief Operations with vectors and quaternions
**/
#ifndef __VECT_OPS__
#define __VECT_OPS__

#include "../params_var.h"

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
float* vox_rotate_vector (const vox_quat, const vox_dot, vox_dot);

/**
   \brief Reverse the vector

   Find vector \f$v^\prime\f$ defined by equation \f$v^\prime + v
   = 0\f$

   \param v a vector to invert
   \param res to store the result
   \return pointer to res
**/
float* vox_vector_inv (const vox_dot, vox_dot);

/**
   \brief Return a conjugate of quaternion

   Return quaternion \f$\overline{q}\f$ defined by equation
   \f$\overline{q}q = {\vert q \vert}^2\f$

   \param q a quaternion to conjugate
   \param res to store the result
   \return pointer to res
**/
float* vox_quat_conj (const vox_quat, vox_quat);

/**
   \brief Return multiplication of 2 quaternions
**/
float* vox_quat_mul (const vox_quat, const vox_quat, vox_quat);

/**
   \brief Return dot product of 2 vectors
**/
float vox_dot_product (const vox_dot, const vox_dot);

#endif
