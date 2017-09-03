#include <assert.h>
#include "copy_helper.h"

// Copy rendered 4x4 squares into flat output array
void copy_squares (square *src, uint32_t *dist, unsigned int ws, unsigned int hs)
{
    int i,j, k;
    int pos = 0, pos_square = 0;
    int w = ws << 2;
    int diff = (w << 2) - w;

    // It's better to handle this properly
    assert (!((size_t)dist & 0xf));
    for (i=0; i<hs; i++)
    {
        for (j=0; j<ws; j+=4)
        {
            int curpos = pos;
            for (k=0; k<4; k++)
            {
                int line = k << 2;
#ifdef SSE_INTRIN
                _mm_stream_si128 ((void*)(dist+curpos),    _mm_load_si128 ((void*)&(src[pos_square  ][line])));
                _mm_stream_si128 ((void*)(dist+curpos+4),  _mm_load_si128 ((void*)&(src[pos_square+1][line])));
                _mm_stream_si128 ((void*)(dist+curpos+8),  _mm_load_si128 ((void*)&(src[pos_square+2][line])));
                _mm_stream_si128 ((void*)(dist+curpos+12), _mm_load_si128 ((void*)&(src[pos_square+3][line])));
#else
                memcpy (dist+curpos,    &src[pos_square  ][line], sizeof (uint32_t)*4);
                memcpy (dist+curpos+4,  &src[pos_square+1][line], sizeof (uint32_t)*4);
                memcpy (dist+curpos+8,  &src[pos_square+2][line], sizeof (uint32_t)*4);
                memcpy (dist+curpos+12, &src[pos_square+3][line], sizeof (uint32_t)*4);
#endif
                curpos += w;
            }
            pos_square += 4;
            pos += 16;
        }
        pos += diff;
    }
}

// Write zeros to squares bypassing the cache (if possible)
#ifdef SSE_INTRIN
void zero_squares (square *ptr, size_t len)
{
    size_t i;
    __m128i zero = _mm_setzero_si128 ();

    for (i=0; i<len; i++)
    {
        _mm_stream_si128 ((void*)&(ptr[i][0]), zero);
        _mm_stream_si128 ((void*)&(ptr[i][4]),  zero);
        _mm_stream_si128 ((void*)&(ptr[i][8]),  zero);
        _mm_stream_si128 ((void*)&(ptr[i][12]), zero);
    }
}
#else
void zero_squares (square *ptr, size_t len)
{
    memset (ptr, 0, len*sizeof(square));
}
#endif
