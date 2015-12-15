#include "vect-ops.h"

#define VOX_N 3

float vox_dot_product (const vox_dot v1, const vox_dot v2)
{
    float res = 0;
    int i;

    for (i=0; i<VOX_N; i++) res += v1[i]*v2[i];
    return res;
}

static float* vox_cross_product (const vox_dot v1, const vox_dot v2, vox_dot res)
{
    res[0] =  v1[1]*v2[2] - v1[2]*v2[1];
    res[1] = -v1[0]*v2[2] + v1[2]*v2[0];
    res[2] =  v1[0]*v2[1] - v1[1]*v2[0];
    return res;
}

float* vox_quat_mul (const vox_quat q1, const vox_quat q2, vox_quat res)
{
    res[0] = q1[1]*q2[2] - q1[2]*q2[1] + q1[0]*q2[3] + q1[3]*q2[0];
    res[1] = q1[2]*q2[0] - q1[0]*q2[2] + q1[1]*q2[3] + q1[3]*q2[1];
    res[2] = q1[0]*q2[1] - q1[1]*q2[0] + q1[2]*q2[3] + q1[3]*q2[2];
    res[3] = q1[3]*q2[3] - q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2];

    return res;
}

float* vox_rotate_vector (const vox_quat base, const vox_dot vector, vox_dot res)
{
    vox_dot tmp, tmp2;
    int i;
    vox_cross_product (base, vector, tmp);
    for (i=0; i<VOX_N; i++) tmp[i] *= 2.0;
    vox_cross_product (base, tmp, tmp2);
    for (i=0; i<VOX_N; i++) res[i] = vector[i] + base[VOX_N]*tmp[i] + tmp2[i];
    
    return res;
}
