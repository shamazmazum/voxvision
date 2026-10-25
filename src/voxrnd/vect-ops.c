#include <math.h>
#include "vect-ops.h"

#define VOX_N 3

static vox_quat vox_quat_conjugate (const vox_quat q) {
    return (vox_quat){-q.x, -q.y, -q.z, q.w};
}


vox_quat vox_quat_mul (const vox_quat q1, const vox_quat q2) {
    float e1 = q1.w, i1 = q1.x, j1 = q1.y, k1 = q1.z;
    float e2 = q2.w, i2 = q2.x, j2 = q2.y, k2 = q2.z;

    float e = e1*e2 - i1*i2 - j1*j2 - k1*k2;
    float i = j1*k2 - k1*j2 + i1*e2 + e1*i2;
    float j = k1*i2 - i1*k2 + j1*e2 + e1*j2;
    float k = i1*j2 - j1*i2 + k1*e2 + e1*k2;

    return (vox_quat){i, j, k, e};
}

vox_dot vox_rotate_vector (const vox_quat base, const vox_dot vector) {
    vox_quat v = (vox_quat)vector;
    v.w = 0;

    vox_quat result = vox_quat_mul(base, vox_quat_mul (v, vox_quat_conjugate (base)));
    return (vox_dot)result;
}

vox_dot vox_dot_normalize (const vox_dot dot) {
    vox_dot p = 2;
    vox_dot tmp = __builtin_elementwise_pow (dot, p);

    float norm = tmp.x + tmp.y + tmp.z;
    return dot / sqrtf (norm);
}

vox_quat vox_quat_normalize (const vox_quat quat) {
    vox_quat p = 2;
    vox_quat tmp = __builtin_elementwise_pow (quat, p);

    float norm = tmp.x + tmp.y + tmp.z + tmp.w;
    return quat / sqrtf (norm);
}
