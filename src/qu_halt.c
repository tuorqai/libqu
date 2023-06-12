//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "qu_halt.h"

//------------------------------------------------------------------------------

void libqu_halt(char const *fmt, ...)
{
    va_list ap;
    char buffer[256];
    char *heap = NULL;

    va_start(ap, fmt);
    int required = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    if ((size_t) required >= sizeof(buffer)) {
        heap = malloc(required + 1);

        if (heap) {
            va_start(ap, fmt);
            vsnprintf(heap, required + 1, fmt, ap);
            va_end(ap);
        }
    }

    fprintf(stderr, "HALT: %s\n", heap ? heap : buffer);
    free(heap);

    abort();
}

//------------------------------------------------------------------------------