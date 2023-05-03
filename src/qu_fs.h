//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_FS_H
#define QU_FS_H

//------------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>

//------------------------------------------------------------------------------

typedef struct libqu_file libqu_file;

libqu_file *libqu_fopen(char const *path);
libqu_file *libqu_mopen(void const *buffer, size_t size);
void libqu_fclose(libqu_file *file);
int64_t libqu_fread(void *buffer, size_t size, libqu_file *file);
int64_t libqu_ftell(libqu_file *file);
int64_t libqu_fseek(libqu_file *file, int64_t offset, int origin);
size_t libqu_file_size(libqu_file *file);
char const *libqu_file_repr(libqu_file *file);

//------------------------------------------------------------------------------

#endif // QU_FS_H

//------------------------------------------------------------------------------