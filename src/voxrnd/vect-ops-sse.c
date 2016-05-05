#include "vect-ops.h"
#ifdef SSE_INTRIN
#include <xmmintrin.h>
#include <pmmintrin.h>

static __v4sf cross_product_ (__v4sf a, __v4sf b)
{
    __v4sf sh1 = _mm_shuffle_ps (b, b, _MM_SHUFFLE (1, 3, 2, 0));
    __v4sf sh2 = _mm_shuffle_ps (a, a, _MM_SHUFFLE (1, 3, 2, 0));
    __v4sf r1 = a * sh1;
    __v4sf r2 = b * sh2;
    __v4sf r = r1 - r2;
    return _mm_shuffle_ps (r, r, _MM_SHUFFLE (1, 3, 2, 0));
}

static __v4sf rotate_vector_ (__v4sf base, __v4sf vect)
{
    /* __v4sf zero = _mm_set_ps1 (0); */
    /* __v4sf base_im = _mm_move_ss (base, zero); */
    __v4sf base_re = _mm_shuffle_ps (base, base, 0);
    __v4sf tmp = cross_product_ (base, vect);
    tmp = tmp * _mm_set_ps1 (2.0);

    __v4sf res = vect + base_re*tmp + cross_product_ (base, tmp);
    return res;
}

static __v4sf quat_mul_ (__v4sf q1, __v4sf q2)
{
    __v4sf re_q1, re_q2, im_q1, im_q2, r1, r2, r3, r4;
    __v4sf sign = _mm_set_epi32 (0x80000000, 0x80000000, 0x80000000, 0);
    __v4sf re = q1*q2;
    re = _mm_xor_ps (re, sign);
    re = _mm_hadd_ps (re, re);
    re = _mm_hadd_ps (re, re);

    re_q1 = _mm_shuffle_ps (q1, q1, 0);
    re_q2 = _mm_shuffle_ps (q2, q2, 0);
    r1 = re_q1 * q2;
    r2 = re_q2 * q1;
    r3 = cross_product_ (q1, q2);
    r4 = r1+r2+r3;
    r4 = _mm_move_ss (r4, re);
    return r4;
}

/*
  This function does not work good with current implementation of the camera
  due to long-read-after-short-write store forwarding stalls on most CPUs,
  but this code is called relatively infrequently, so just leave it as is now.
*/
void vox_quat_mul (const vox_quat q1, const vox_quat q2, vox_quat res)
{
    __v4sf r = quat_mul_ (_mm_load_ps (q1), _mm_load_ps(q2));
    _mm_store_ps (res, r);
}

void vox_rotate_vector (const vox_quat base, const vox_dot vector, vox_dot res)
{
    __v4sf vect = _mm_slli_si128 (_mm_load_ps (vector), 4);
    __v4sf r = rotate_vector_ (_mm_load_ps (base), vect);
    r = _mm_srli_si128 (r, 4);
    _mm_store_ps (res, r);
}
#endif
