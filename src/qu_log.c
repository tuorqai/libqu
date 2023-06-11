//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <libqu.h>
#include "qu_log.h"

//------------------------------------------------------------------------------

void libqu_log(libqu_log_level log_level, char const *fmt, ...)
{
    char *labels[] = {
        [LIBQU_DEBUG]   = "DBG ",
        [LIBQU_INFO]    = "INFO",
        [LIBQU_WARNING] = "WARN",
        [LIBQU_ERROR]   = "ERR ",
    };

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

    fprintf((log_level == LIBQU_ERROR) ? stderr : stdout,
        "%8.3f [%s] %s", qu_get_time_mediump(), labels[log_level], heap ?: buffer);

    free(heap);
}

//------------------------------------------------------------------------------
