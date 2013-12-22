#include <xmmintrin.h>
#include <pmmintrin.h>
#include "vect-ops.h"

static __v4sf conj_ (__v4sf q)
{
    return _mm_xor_ps (q, (__v4sf)_mm_set_epi32 (0, 0x80000000, 0x80000000, 0x80000000));
}

static __v4sf quat_re (__v4sf q) {return _mm_shuffle_ps (q, q, 255);}
static __v4sf quat_im (__v4sf q)
{
    return _mm_and_ps (q, (__v4sf)_mm_set_epi32 (0, 0xffffffff, 0xffffffff, 0xffffffff));
}

static __v4sf dp (__v4sf v1, __v4sf v2)
{
    __v4sf tmp = v1*v2;
    tmp = __builtin_ia32_haddps (tmp,tmp);
    tmp = __builtin_ia32_haddps (tmp,tmp);
    return tmp;
}

static __v4sf cp (__v4sf v1, __v4sf v2)
{
    __v4sf m1 = _mm_shuffle_ps (v1, v1, _MM_SHUFFLE (3, 0, 2, 1)) * \
                _mm_shuffle_ps (v2, v2, _MM_SHUFFLE (3, 1, 0, 2));

    __v4sf m2 = _mm_shuffle_ps (v1, v1, _MM_SHUFFLE (3, 1, 0, 2)) *   \
                _mm_shuffle_ps (v2, v2, _MM_SHUFFLE (3, 0, 2, 1));
    
    return m1-m2;
}

float* vox_rotate_vector (const vox_quat base, const vox_dot vector, vox_dot res)
{
    __v4sf basev = _mm_load_ps (base);
    __v4sf vect = _mm_load_ps (vector);

    // Just to be sure, what we do not have any junk in real slot
    vect = quat_im (vect);

    __v4sf axis = quat_im (basev);
    __v4sf tmp = _mm_set_ps1 (2.0) * cp (axis, vect);

    __v4sf resv = vect + quat_re (basev) * tmp + cp (axis, tmp);

    _mm_store_ps (res, resv);
    return res;
}

float* vox_vector_inv (const vox_dot v, vox_dot res) {_mm_store_ps (res, conj_ (_mm_load_ps (v))); return res;}
float* vox_quat_conj (const vox_quat v, vox_quat res) {_mm_store_ps (res, conj_ (_mm_load_ps (v))); return res;}

float* vox_quat_mul (const vox_quat q1, const vox_quat q2, vox_quat res)
{
    __v4sf v1 = _mm_load_ps (q1);
    __v4sf v2 = _mm_load_ps (q2);
    __v4sf tmp1, tmp2, tmp3, tmp4, resv;
    __v4sf sign_mask = (__v4sf)_mm_set_epi32 (0x80000000, 0, 0, 0);

    tmp1 = _mm_shuffle_ps (v1, v1, _MM_SHUFFLE (3, 0, 2, 1)) *      \
        _mm_shuffle_ps (v2, v2, _MM_SHUFFLE (3, 1, 0, 2));

    tmp2 = -_mm_shuffle_ps (v1, v1, _MM_SHUFFLE (0, 1, 0, 2)) *      \
        _mm_shuffle_ps (v2, v2, _MM_SHUFFLE (0, 0, 2, 1));

    tmp3 = _mm_shuffle_ps (v1, v1, _MM_SHUFFLE (1, 2, 1, 0)) *    \
        _mm_shuffle_ps (v2, v2, _MM_SHUFFLE (1, 3, 3, 3));
    tmp3 = _mm_xor_ps (tmp3, sign_mask);
    
    tmp4 = _mm_shuffle_ps (v1, v1, _MM_SHUFFLE (2, 3, 3, 3)) *    \
        _mm_shuffle_ps (v2, v2, _MM_SHUFFLE (2, 2, 1, 0));
    tmp4 = _mm_xor_ps (tmp4, sign_mask);

    resv = tmp1+tmp2+tmp3+tmp4;
    _mm_store_ps (res, resv);
    return res;
}

float vox_dot_product (const vox_dot v1, const vox_dot v2)
{
    __v4sf vect1 = _mm_load_ps (v1);
    __v4sf vect2 = _mm_load_ps (v2);
    
    // Strip junk
    vect1 = quat_im (vect1);
    vect2 = quat_im (vect2);

    __v4sf prod = dp (vect1, vect2);
    return prod[0];
}