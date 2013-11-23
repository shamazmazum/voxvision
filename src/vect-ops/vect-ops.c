#include "vect-ops.h"

float* vector_inv (vox_dot vect, vox_dot res)
{
    int i;
    for (i=0; i<VOX_N; i++) res[i] = -vect[i];
    return res;
}

float* quat_conj (vox_quat quat, vox_quat res)
{
    int i;
    for (i=0; i<3; i++) res[i] = -quat[i];
    return res;
}

float dot_product (vox_dot v1, vox_dot v2)
{
    float res = 0;
    int i;

    for (i=0; i<VOX_N; i++) res += v1[i]*v2[i];
    return res;
}

static float* cross_product (vox_dot v1, vox_dot v2, vox_dot res)
{
    res[0] =  v1[1]*v2[2] - v1[2]*v2[1];
    res[1] = -v1[0]*v2[2] + v1[2]*v2[0];
    res[2] =  v1[0]*v2[1] - v1[1]*v2[0];
    return res;
}

float* rotate_vector (vox_quat base, vox_dot vector, vox_dot res)
{
    vox_dot tmp, tmp2;
    int i;
    cross_product (base, vector, tmp);
    for (i=0; i<VOX_N; i++) tmp[i] *= 2.0;
    cross_product (base, tmp, tmp2);
    for (i=0; i<VOX_N; i++) res[i] = vector[i] + base[VOX_N]*tmp[i] + tmp2[i];
    
    return res;
}
