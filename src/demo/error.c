#include <stdio.h>
#include "error.h"

int gerror;

static const char *error_strings[] = {
    "No memory",
    "Size mismatch",
    "No access",
    "Unknown"
};
 
const char* get_error_string (int code)
{
    const char *res = NULL;
    if ((code >= 0) && (code < ERRMAX)) res = error_strings[code-1];
    return res;
}
