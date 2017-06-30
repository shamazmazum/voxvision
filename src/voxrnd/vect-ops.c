#include <math.h>
#include "vect-ops.h"
#ifndef SSE_INTRIN

#define VOX_N 3

static void vox_cross_product (const vox_dot v1, const vox_dot v2, vox_dot res)
{
    res[0] =  v1[1]*v2[2] - v1[2]*v2[1];
    res[1] = -v1[0]*v2[2] + v1[2]*v2[0];
    res[2] =  v1[0]*v2[1] - v1[1]*v2[0];
}

void vox_quat_mul (const vox_quat q1, const vox_quat q2, vox_quat res)
{
    res[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    res[1] = q1[2]*q2[3] - q1[3]*q2[2] + q1[1]*q2[0] + q1[0]*q2[1];
    res[2] = q1[3]*q2[1] - q1[1]*q2[3] + q1[2]*q2[0] + q1[0]*q2[2];
    res[3] = q1[1]*q2[2] - q1[2]*q2[1] + q1[3]*q2[0] + q1[0]*q2[3];
}

void vox_rotate_vector (const vox_quat base, const vox_dot vector, vox_dot res)
{
    vox_dot tmp, tmp2;
    int i;
    vox_cross_product ((float*)(base+1), vector, tmp);
    for (i=0; i<VOX_N; i++) tmp[i] *= 2.0;
    vox_cross_product ((float*)(base+1), tmp, tmp2);
    for (i=0; i<VOX_N; i++) res[i] = vector[i] + base[0]*tmp[i] + tmp2[i];
}

void vox_quat_normalize (vox_quat quat)
{
    int i;
    float norm, sum = 0;

    for (i=0; i<4; i++) sum += quat[i]*quat[i];
    norm = sqrtf (sum);

    for (i=0; i<4; i++) quat[i] /= norm;
}

void vox_dot_normalize (vox_quat quat)
{
    int i;
    float norm, sum = 0;

    for (i=0; i<3; i++) sum += quat[i]*quat[i];
    norm = sqrtf (sum);

    for (i=0; i<3; i++) quat[i] /= norm;
}
#endif
