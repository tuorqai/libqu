//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include "qu_util.h"

//------------------------------------------------------------------------------

char *libqu_strdup(char const *str)
{
    size_t size = strlen(str) + 1;
    char *dup = malloc(size);

    if (dup) {
        memcpy(dup, str, size);
    }

    return dup;
}

void libqu_make_circle(float x, float y, float radius, float *data, int num_verts)
{
    float angle = QU_DEG2RAD(360.f / num_verts);
    
    for (int i = 0; i < num_verts; i++) {
        data[2 * i + 0] = x + (radius * cosf(i * angle));
        data[2 * i + 1] = y + (radius * sinf(i * angle));
    }
}

//------------------------------------------------------------------------------
