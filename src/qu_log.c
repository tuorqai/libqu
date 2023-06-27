//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include "qu.h"

//------------------------------------------------------------------------------

void qu_log(qu_log_level log_level, char const *module, char const *fmt, ...)
{
    char *labels[] = {
        [QU_LOG_LEVEL_DEBUG]   = "DBG ",
        [QU_LOG_LEVEL_INFO]    = "INFO",
        [QU_LOG_LEVEL_WARNING] = "WARN",
        [QU_LOG_LEVEL_ERROR]   = "ERR ",
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

    if (module[0] == '?') {
        for (int i = strlen(module) - 1; i >= 0; i--) {
            if (module[i] == '/' || module[i] == '\\') {
                module = module + i + 1;
                break;
            }
        }
    }

    fprintf((log_level == QU_LOG_LEVEL_ERROR) ? stderr : stdout,
            "(%8.3f) [%s] %s: %s", qu_get_time_mediump(),
            labels[log_level], module, heap ? heap : buffer);

    free(heap);
}
