#ifndef __VECT_OPS__
#define __VECT_OPS__

#include "../params_var.h"

float* vox_rotate_vector (const vox_quat, const vox_dot, vox_dot);
float* vox_vector_inv (const vox_dot, vox_dot);
float* vox_quat_conj (const vox_quat, vox_quat);

float vox_dot_product (const vox_dot, const vox_dot);

#endif
