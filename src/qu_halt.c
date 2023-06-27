//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include "qu.h"

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

#ifdef _WIN32
    size_t len = strlen(heap ? heap : buffer);
    LPTSTR msg = malloc((len + 1) * sizeof(TCHAR));

    if (msg) {
#ifdef UNICODE
        mbstowcs(msg, heap ? heap : buffer, len);
#else
        strncpy(msg, heap ? heap : buffer, len);
#endif
        msg[len] = 0;
        MessageBox(NULL, msg, TEXT("libqu error"), MB_OK | MB_ICONERROR);
        free(msg);
    }
#endif

    free(heap);

    abort();
}

//------------------------------------------------------------------------------