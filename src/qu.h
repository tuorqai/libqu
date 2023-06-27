//------------------------------------------------------------------------------
// Copyright (c) 2021-2023 tuorqai
// 
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//------------------------------------------------------------------------------
// qu.h: internal header file
//------------------------------------------------------------------------------

#ifndef QU_PRIVATE_H_INC
#define QU_PRIVATE_H_INC

//------------------------------------------------------------------------------

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <libqu.h>

//------------------------------------------------------------------------------
// Math

void libqu_mat4_identity(float *m);
void libqu_mat4_copy(float *dst, float *src);
void libqu_mat4_multiply(float *m, float const *n);
void libqu_mat4_ortho(float *m, float l, float r, float b, float t);
void libqu_mat4_translate(float *m, float x, float y, float z);
void libqu_mat4_scale(float *m, float x, float y, float z);
void libqu_mat4_rotate(float *m, float rad, float x, float y, float z);
void libqu_mat4_inverse(float *dst, float const *src);
qu_vec2f libqu_mat4_transform_point(float const *m, qu_vec2f p);

//------------------------------------------------------------------------------
// Util

char *libqu_strdup(char const *str);
void libqu_make_circle(float x, float y, float radius, float *data, int num_verts);

//------------------------------------------------------------------------------
// Log

#ifndef QU_MODULE
#   define QU_MODULE    "?"__FILE__
#endif

typedef enum
{
    QU_LOG_LEVEL_DEBUG,
    QU_LOG_LEVEL_INFO,
    QU_LOG_LEVEL_WARNING,
    QU_LOG_LEVEL_ERROR,
} qu_log_level;

void qu_log(qu_log_level log_level, char const *module, char const *fmt, ...);

#if defined(NDEBUG)
#   define libqu_debug
#else
#   define libqu_debug(...) \
        qu_log(QU_LOG_LEVEL_DEBUG, QU_MODULE, __VA_ARGS__)
#endif

#define libqu_info(...) \
    qu_log(QU_LOG_LEVEL_INFO, QU_MODULE, __VA_ARGS__)

#define libqu_warning(...) \
    qu_log(QU_LOG_LEVEL_WARNING, QU_MODULE, __VA_ARGS__)

#define libqu_error(...) \
    qu_log(QU_LOG_LEVEL_ERROR, QU_MODULE, __VA_ARGS__)

//------------------------------------------------------------------------------
// Halt

QU_NO_RET void libqu_halt(char const *fmt, ...);

//------------------------------------------------------------------------------
// Array

typedef struct libqu_array libqu_array;

libqu_array *libqu_create_array(size_t element_size, void (*dtor)(void *));
void libqu_destroy_array(libqu_array *array);

int32_t libqu_array_add(libqu_array *array, void *data);
void libqu_array_remove(libqu_array *array, int32_t id);
void *libqu_array_get(libqu_array *array, int32_t id);

//------------------------------------------------------------------------------
// FS

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

#endif // QU_PRIVATE_H_INC
