#ifndef __VECT_OPS__
#define __VECT_OPS__

#include "../params_var.h"

float* rotate_vector (vox_quat, vox_dot, vox_dot);
float* vector_inv (vox_dot, vox_dot);
float* quat_conj (vox_quat, vox_quat);

float dot_product (vox_dot, vox_dot);

#endif
