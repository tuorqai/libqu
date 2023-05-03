//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_ARRAY_H
#define QU_ARRAY_H

//------------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>

//------------------------------------------------------------------------------

typedef struct libqu_array libqu_array;

//------------------------------------------------------------------------------

libqu_array *libqu_create_array(size_t element_size, void (*dtor)(void *));
void libqu_destroy_array(libqu_array *array);

int32_t libqu_array_add(libqu_array *array, void *data);
void libqu_array_remove(libqu_array *array, int32_t id);
void *libqu_array_get(libqu_array *array, int32_t id);

//------------------------------------------------------------------------------

#endif // QU_ARRAY_H

//------------------------------------------------------------------------------
