//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include "qu.h"

//------------------------------------------------------------------------------

typedef enum file_source
{
    SOURCE_FS,
    SOURCE_MEMORY,
} file_source;

struct libqu_file
{
    file_source source;

    union {
        struct {
            FILE *stream;
        } fs;

        struct {
            uint8_t const *buffer;
            size_t offset;
        } memory;
    };

    size_t size;
    char repr[256];
};

libqu_file *libqu_fopen(char const *path)
{
    libqu_file *file = calloc(1, sizeof(libqu_file));

    if (!file) {
        return NULL;
    }

    file->source = SOURCE_FS;
    file->fs.stream = fopen(path, "rb");

    if (!file->fs.stream) {
        free(file);
        return NULL;
    }

    fseek(file->fs.stream, 0, SEEK_END);
    file->size = (size_t) ftell(file->fs.stream);
    fseek(file->fs.stream, 0, SEEK_SET);

    strncpy(file->repr, path, sizeof(file->repr) - 1);

    return file;
}

libqu_file *libqu_mopen(void const *buffer, size_t size)
{
    libqu_file *file = calloc(1, sizeof(libqu_file));

    if (!file) {
        return NULL;
    }

    file->source = SOURCE_MEMORY;
    file->memory.buffer = (uint8_t const *) buffer;
    file->memory.offset = 0;
    file->size = size;

    snprintf(file->repr, sizeof(file->repr) - 1, "0x%p", buffer);

    return file;
}

void libqu_fclose(libqu_file *file)
{
    if (file->source == SOURCE_FS) {
        fclose(file->fs.stream);
    }

    free(file);
}

int64_t libqu_fread(void *buffer, size_t size, libqu_file *file)
{
    int64_t bytes = 0;

    switch (file->source) {
    case SOURCE_FS:
        bytes = fread(buffer, 1, size, file->fs.stream);
        break;
    case SOURCE_MEMORY:
        if ((file->memory.offset + size) <= file->size) {
            bytes = size;
        } else {
            bytes = file->size - file->memory.offset;
        }
        memcpy(buffer, file->memory.buffer + file->memory.offset, bytes);
        break;
    default:
        break;
    }

    return bytes;
}

int64_t libqu_ftell(libqu_file *file)
{
    switch (file->source) {
    case SOURCE_FS:
        return ftell(file->fs.stream);
    case SOURCE_MEMORY:
        return file->memory.offset;
    default:
        return 0;
    }
}

int64_t libqu_fseek(libqu_file *file, int64_t offset, int origin)
{
    int64_t abs_offset = offset;

    switch (file->source) {
    case SOURCE_FS:
        return fseek(file->fs.stream, offset, origin);
    case SOURCE_MEMORY:
        if (origin == SEEK_CUR) {
            abs_offset = file->memory.offset + offset;
        } else if (origin == SEEK_END) {
            abs_offset = file->size + offset;
        }

        if (abs_offset < 0 || abs_offset > (int64_t) file->size) {
            return -1;
        }

        file->memory.offset = abs_offset;
        return 0;
    default:
        return -1;
    }
}

size_t libqu_file_size(libqu_file *file)
{
    return file->size;
}

char const *libqu_file_repr(libqu_file *file)
{
    return file->repr;
}

//------------------------------------------------------------------------------
