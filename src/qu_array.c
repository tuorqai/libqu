//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "qu_array.h"
#include "qu_log.h"

//------------------------------------------------------------------------------

#define FLAG_USED                       (0x01)

//------------------------------------------------------------------------------

struct control
{
    uint8_t flags;
    uint8_t gen;
};

struct libqu_array
{
    size_t element_size;
    void (*dtor)(void *);

    size_t size;
    size_t used;
    int64_t last_free_index;
    struct control *control;
    void *data;
};

//------------------------------------------------------------------------------

static int32_t encode_id(int index, int gen)
{
    // Identifier layout:
    // - bits 0 to 17 hold index number
    // - bits 18 to 23 should be 1
    // - bits 24 to 30 hold generation number
    // - bit 31 is unused

    return ((gen & 0x7F) << 24) | 0x00FC0000 | (index & 0x3FFFF);
}

static bool decode_id(int32_t id, int *index, int *gen)
{
    if ((id & 0x00FC0000) == 0) {
        return false;
    }

    *index = id & 0x3FFFF;
    *gen = (id >> 24) & 0x7F;

    return true;
}

static int find_index(libqu_array *array)
{
    if (array->last_free_index >= 0) {
        int index = array->last_free_index;
        array->last_free_index = -1;
        return index;
    }

    for (size_t i = 0; i < array->size; i++) {
        if ((array->control[i].flags & FLAG_USED) == 0) {
            return i;
        }
    }

    return -1;
}

static bool grow_array(libqu_array *array)
{
    size_t next_size = array->size ? (array->size * 2) : 16;
    struct control *next_control = realloc(array->control, sizeof(struct control) * next_size);
    void *next_data = realloc(array->data, array->element_size * next_size);

    if (!next_control || !next_data) {
        return false;
    }

    libqu_debug("grow_array(): %d -> %d\n", array->size, next_size);

    for (size_t i = array->size; i < next_size; i++) {
        next_control[i].flags = 0;
        next_control[i].gen = 0;
    }

    array->size = next_size;
    array->control = next_control;
    array->data = next_data;

    return true;
}

static void *get_data(libqu_array *array, int index)
{
    return ((uint8_t *) array->data) + (array->element_size * index);
}

//------------------------------------------------------------------------------

libqu_array *libqu_create_array(size_t element_size, void (*dtor)(void *))
{
    libqu_array *array = malloc(sizeof(libqu_array));

    if (!array) {
        return NULL;
    }

    *array = (libqu_array) {
        .element_size = element_size,
        .dtor = dtor,
        .size = 0,
        .used = 0,
        .last_free_index = -1,
        .control = NULL,
        .data = NULL,
    };

    return array;
}

void libqu_destroy_array(libqu_array *array)
{
    if (!array) {
        return;
    }

    if (array->dtor) {
        for (size_t i = 0; i < array->size; i++) {
            if (array->control[i].flags & FLAG_USED) {
                array->dtor(get_data(array, i));
            }
        }
    }

    free(array->control);
    free(array->data);
    free(array);
}

int32_t libqu_array_add(libqu_array *array, void *data)
{
    if (array->used == array->size) {
        if (!grow_array(array)) {
            if (array->dtor) {
                array->dtor(data);
            }

            return 0;
        }
    }

    int index = find_index(array);

    if (index == -1) {
        if (array->dtor) {
            array->dtor(data);
        }
        
        return 0;
    }

    ++array->used;
    array->control[index].flags = FLAG_USED;
    memcpy((uint8_t *) get_data(array, index), data, array->element_size);

    return encode_id(index, array->control[index].gen);
}

void libqu_array_remove(libqu_array *array, int32_t id)
{
    int index, gen;

    if (!decode_id(id, &index, &gen) || array->control[index].gen != gen) {
        return;
    }

    --array->used;

    if (array->dtor) {
        array->dtor(get_data(array, index));
    }

    array->control[index].flags = 0x00;
    array->control[index].gen = (array->control[index].gen + 1) & 0x7F;

    array->last_free_index = index;
}

void *libqu_array_get(libqu_array *array, int32_t id)
{
    if (id == 0) {
        return NULL;
    }

    int index, gen;

    if (!decode_id(id, &index, &gen) || array->control[index].gen != gen) {
        return NULL;
    }

    return get_data(array, index);
}

//------------------------------------------------------------------------------
