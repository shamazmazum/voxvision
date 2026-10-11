#include <math.h>
#include "vect-ops.h"

#define VOX_N 3

static void vox_quat_conjugate (const vox_quat q, vox_quat res)
{
    res[0] = +q[0];
    res[1] = -q[1];
    res[2] = -q[2];
    res[3] = -q[3];
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
    vox_quat q, tmp;
    q[0] = 0;
    q[1] = vector[0];
    q[2] = vector[1];
    q[3] = vector[2];

    vox_quat_conjugate (base, tmp);
    vox_quat_mul (base, q, q);
    vox_quat_mul (q, tmp, q);

    res[0] = q[1];
    res[1] = q[2];
    res[2] = q[3];
}

void vox_rotate_vector_q (const vox_quat base, const vox_quat vector, vox_quat res)
{
    vox_quat q, tmp;
    vox_quat_conjugate (base, tmp);
    vox_quat_mul (base, vector, q);
    vox_quat_mul (q, tmp, res);
}

void vox_quat_normalize (vox_quat quat)
{
    int i;
    float norm, sum = 0;

    for (i=0; i<4; i++) sum += quat[i]*quat[i];
    norm = sqrtf (sum);

    for (i=0; i<4; i++) quat[i] /= norm;
}

void vox_dot_normalize (vox_dot dot)
{
    int i;
    float norm, sum = 0;

    for (i=0; i<3; i++) sum += dot[i]*dot[i];
    norm = sqrtf (sum);

    for (i=0; i<3; i++) dot[i] /= norm;
}

void vox_quat_set_identity (vox_quat quat)
{
    memset (quat, 0, sizeof (vox_quat));
    quat[0] = 1;
}
