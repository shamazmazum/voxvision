#include <npy_array.h>
#include <stdlib.h>
#include "tree.h"
#include "datareader.h"

struct vox_node* vox_read_data (const char *filename,
                                bool (^test)(uint32_t sample, uint8_t bps),
                                const char **error) {
    npy_array_t *array = npy_array_load (filename);
    struct vox_node *tree = NULL;
    vox_dot *set = NULL;
    uint8_t bps;
    size_t d, h, w, total;
    size_t array_count = 0, set_count = 0;
    void *ptr;
    uint64_t sample;

    if (array == NULL) {
        if (error != NULL) {
            *error = "Cannot open Numpy array";
        }
        goto done;
    }

    if (array->ndim != 3) {
        if (error != NULL) {
            *error = "Array must be 3-dimensional";
        }
        goto done;
    }

    if (array->typechar != 'b' && array->typechar != 'u') {
        if (error != NULL) {
            *error = "Unknown array format";
        }
        goto done;
    }

    if ((array->typechar == 'b') && (array->elem_size == 1)) {
        bps = 1;
    } else if ((array->typechar == 'u') && (array->elem_size == 1)) {
        bps = 8;
    } else if ((array->typechar == 'u') && (array->elem_size == 2)) {
        bps = 16;
    } else if ((array->typechar == 'u') && (array->elem_size == 4)) {
        bps = 32;
    } else {
        if (error != NULL) {
            *error = "Unknown array format";
        }
        goto done;
    }

    ptr = array->data;
    d = array->shape[0];
    h = array->shape[1];
    w = array->shape[2];
    total = d * w * h;

    if (total > 500*500*500) {
        if (error != NULL) {
            *error = "This array is too big for the datareader to handle, sorry";
        }
        goto done;
    }

    set = malloc (sizeof (vox_dot) * total);

    for (size_t i = 0; i < d; i++) {
        for (size_t j = 0; j < h; j++) {
            for (size_t k = 0; k < w; k++) {
                switch (bps) {
                case 1:
                case 8:
                    sample = ((uint8_t*)ptr)[array_count];
                    break;
                case 16:
                    sample = ((uint16_t*)ptr)[array_count];
                    break;
                case 32:
                    sample = ((uint32_t*)ptr)[array_count];
                    break;
                case 64:
                    sample = ((uint64_t*)ptr)[array_count];
                    break;                    
                }
                if (test (sample, bps)) {
                    vox_dot point = (vox_dot) {i - d/2.0, j - h/2.0, k - w/2.0};
                    point *= vox_voxel;
                    set[set_count] = point;
                    set_count++;
                }
                array_count++;
            }
        }
    }

    tree = vox_make_tree (set, set_count);
    if (error != NULL) {
        *error = "OK";
    }

done:
    if (array != NULL) {
        npy_array_free (array);
    }

    if (set != NULL) {
        free (set);
    }

    return tree;
}
