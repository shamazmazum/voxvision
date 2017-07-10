#include <math.h>
#include "vect-ops.h"
#ifndef SSE_INTRIN

#define VOX_N 3

static void vox_cross_product (const vox_dot v1, const vox_dot v2, vox_dot res)
{
    float i1 = v1[0], j1 = v1[1], k1 = v1[2];
    float i2 = v2[0], j2 = v2[1], k2 = v2[2];

    res[0] =  j1*k2 - k1*j2;
    res[1] = -i1*k2 + k1*i2;
    res[2] =  i1*j2 - j1*i2;
}

void vox_quat_mul (const vox_quat q1, const vox_quat q2, vox_quat res)
{
    float e1 = q1[0], i1 = q1[1], j1 = q1[2], k1 = q1[3];
    float e2 = q2[0], i2 = q2[1], j2 = q2[2], k2 = q2[3];

    res[0] = e1*e2 - i1*i2 - j1*j2 - k1*k2;
    res[1] = j1*k2 - k1*j2 + i1*e2 + e1*i2;
    res[2] = k1*i2 - i1*k2 + j1*e2 + e1*j2;
    res[3] = i1*j2 - j1*i2 + k1*e2 + e1*k2;
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

void vox_quat_set_identity (vox_quat quat)
{
    memset (quat, 0, sizeof (vox_quat));
    quat[0] = 1;
}
#endif
