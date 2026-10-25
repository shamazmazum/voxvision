#include <assert.h>
#include "copy-helper.h"

// Copy rendered 4x4 squares into flat output array
void copy_squares (square *src, uint32_t *dist, unsigned int ws, unsigned int hs) {
    unsigned int i,j, k;
    unsigned int pos = 0, pos_square = 0;
    unsigned int w = ws << 2;
    unsigned int diff = (w << 2) - w;

    // It's better to handle this properly
    assert (!((size_t)dist & 0xf));
    for (i=0; i<hs; i++) {
        for (j=0; j<ws; j+=4) {
            int curpos = pos;
            for (k=0; k<4; k++) {
                int line = k << 2;
                memcpy (dist+curpos,    &src[pos_square  ][line], sizeof (uint32_t)*4);
                memcpy (dist+curpos+4,  &src[pos_square+1][line], sizeof (uint32_t)*4);
                memcpy (dist+curpos+8,  &src[pos_square+2][line], sizeof (uint32_t)*4);
                memcpy (dist+curpos+12, &src[pos_square+3][line], sizeof (uint32_t)*4);
                curpos += w;
            }
            pos_square += 4;
            pos += 16;
        }
        pos += diff;
    }
}

void zero_squares (square *ptr, size_t len) {
    memset (ptr, 0, len*sizeof(square));
}
