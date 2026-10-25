#include "object.h"

void vox_method_void_dummy (void *obj, ...) {
}

float vox_method_float_dummy (void *obj, ...) {
    return 0;
}

vox_dot vox_method_dot_dummy (void *obj, ...) {
    return (vox_dot) {0, 0, 0, 0};
}
