#include <stdio.h>
#include "object.h"

void vox_method_void_dummy (void *obj, ...) {}
float vox_method_float_dummy (void *obj, ...) {return 0;}
void vox_method_dot_dummy (void *obj, vox_dot dot, ...)
{
    vox_dot_set (dot, 0, 0, 0);
}
