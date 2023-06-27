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

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <tchar.h>
#   include <windows.h>
#endif

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#endif

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
// Image loader

typedef struct
{
    int width;
    int height;
    int channels;
    unsigned char *pixels;
} libqu_image;

libqu_image *libqu_load_image(libqu_file *file);
void libqu_delete_image(libqu_image *image);

//------------------------------------------------------------------------------
// Sound loader

typedef struct
{
    int16_t num_channels;
    int64_t num_samples;
    int64_t sample_rate;
    libqu_file *file;
    void *data;
} libqu_sound;

libqu_sound *libqu_open_sound(libqu_file *file);
void libqu_close_sound(libqu_sound *sound);
int64_t libqu_read_sound(libqu_sound *sound, int16_t *samples, int64_t max_samples);
void libqu_seek_sound(libqu_sound *sound, int64_t sample_offset);

//------------------------------------------------------------------------------
// Platform

typedef struct libqu_thread libqu_thread;
typedef struct libqu_mutex libqu_mutex;
typedef intptr_t(*libqu_thread_func)(void *);

void libqu_platform_initialize(void);
void libqu_platform_terminate(void);

float libqu_get_time_mediump(void);
double libqu_get_time_highp(void);

libqu_thread *libqu_create_thread(char const *name, libqu_thread_func func, void *arg);
void libqu_detach_thread(libqu_thread *thread);
intptr_t libqu_wait_thread(libqu_thread *thread);

libqu_mutex *libqu_create_mutex(void);
void libqu_destroy_mutex(libqu_mutex *mutex);
void libqu_lock_mutex(libqu_mutex *mutex);
void libqu_unlock_mutex(libqu_mutex *mutex);

void libqu_sleep(double seconds);

//------------------------------------------------------------------------------
// Core

typedef enum
{
    LIBQU_GC_GL,
    LIBQU_GC_GLES,
    LIBQU_GC_NONE,
} libqu_gc;

typedef struct
{
    void (*initialize)(qu_params const *params);
    void (*terminate)(void);
    bool (*is_initialized)(void);
    bool (*process)(void);
    void (*present)(void);
    libqu_gc(*get_gc)(void);
    bool (*gl_check_extension)(char const *name);
    void *(*gl_proc_address)(char const *name);

    bool const *(*get_keyboard_state)(void);
    bool (*is_key_pressed)(qu_key key);

    uint8_t(*get_mouse_button_state)(void);
    bool (*is_mouse_button_pressed)(qu_mouse_button button);
    qu_vec2i(*get_mouse_cursor_position)(void);
    qu_vec2i(*get_mouse_cursor_delta)(void);
    qu_vec2i(*get_mouse_wheel_delta)(void);

    bool (*is_joystick_connected)(int joystick);
    char const *(*get_joystick_id)(int joystick);
    int (*get_joystick_button_count)(int joystick);
    int (*get_joystick_axis_count)(int joystick);
    char const *(*get_joystick_button_id)(int joystick, int button);
    char const *(*get_joystick_axis_id)(int joystick, int axis);
    bool (*is_joystick_button_pressed)(int joystick, int button);
    float (*get_joystick_axis_value)(int joystick, int axis);

    void (*on_key_pressed)(qu_key_fn fn);
    void (*on_key_repeated)(qu_key_fn fn);
    void (*on_key_released)(qu_key_fn fn);
    void (*on_mouse_button_pressed)(qu_mouse_button_fn fn);
    void (*on_mouse_button_released)(qu_mouse_button_fn fn);
    void (*on_mouse_cursor_moved)(qu_mouse_cursor_fn fn);
    void (*on_mouse_wheel_scrolled)(qu_mouse_wheel_fn fn);

    float (*get_time_mediump)(void);
    double (*get_time_highp)(void);
} libqu_core;

void libqu_construct_null_core(libqu_core *core);

#if defined(__ANDROID__)
void libqu_construct_android_core(libqu_core *core);
#endif

#if defined(__EMSCRIPTEN__)
void libqu_construct_emscripten_core(libqu_core *core);
#endif

#if defined(__unix__)
void libqu_construct_unix_core(libqu_core *core);
#endif

#if defined(_WIN32)
void libqu_construct_win32_core(libqu_core *core);
#endif

//------------------------------------------------------------------------------
// Graphics

typedef struct
{
    void (*initialize)(qu_params const *params);
    void (*terminate)(void);
    bool (*is_initialized)(void);
    void (*swap)(void);
    void (*notify_display_resize)(int width, int height);
    qu_vec2i(*conv_cursor)(qu_vec2i position);
    qu_vec2i(*conv_cursor_delta)(qu_vec2i position);

    void (*set_view)(float x, float y, float w, float h, float rotation);
    void (*reset_view)(void);

    void (*push_matrix)(void);
    void (*pop_matrix)(void);
    void (*translate)(float x, float y);
    void (*scale)(float x, float y);
    void (*rotate)(float degrees);

    void (*clear)(qu_color color);
    void (*draw_point)(float x, float y, qu_color color);
    void (*draw_line)(float ax, float ay, float bx, float by, qu_color color);
    void (*draw_triangle)(float ax, float ay, float bx, float by, float cx,
                          float cy, qu_color outline, qu_color fill);
    void (*draw_rectangle)(float x, float y, float w, float h, qu_color outline,
                           qu_color fill);
    void (*draw_circle)(float x, float y, float radius, qu_color outline,
                        qu_color fill);

    int32_t(*create_texture)(int w, int h, int channels);
    void (*update_texture)(int32_t texture_id, int x, int y, int w, int h,
                           uint8_t const *pixels);
    int32_t(*load_texture)(libqu_file *file);
    void (*delete_texture)(int32_t texture_id);
    void (*set_texture_smooth)(int32_t texture_id, bool smooth);
    void (*draw_texture)(int32_t texture_id, float x, float y, float w,
                         float h);
    void (*draw_subtexture)(int32_t texture_id, float x, float y, float w,
                            float h, float rx, float ry, float rw, float rh);

    void (*draw_text)(int32_t texture_id, qu_color color, float const *data,
                      int count);

    int32_t(*create_surface)(int width, int height);
    void (*delete_surface)(int32_t id);
    void (*set_surface)(int32_t id);
    void (*reset_surface)(void);
    void (*draw_surface)(int32_t id, float x, float y, float w, float h);
} libqu_graphics;

void libqu_construct_null_graphics(libqu_graphics *graphics);

#ifndef QU_DISABLE_GL
void libqu_construct_gl2_graphics(libqu_graphics *graphics);
#endif

#ifndef QU_DISABLE_GLES2
void libqu_construct_gles2_graphics(libqu_graphics *graphics);
#endif

//------------------------------------------------------------------------------
// Text

void libqu_initialize_text(libqu_graphics *graphics);
void libqu_terminate_text(void);
int32_t libqu_load_font(libqu_file *file, float pt);
void libqu_delete_font(int32_t font_id);
void libqu_draw_text(int32_t font_id, float x, float y, qu_color color, char const *text);

//------------------------------------------------------------------------------
// Audio

typedef struct
{
    void (*initialize)(qu_params const *params);
    void (*terminate)(void);
    bool (*is_initialized)(void);

    void (*set_master_volume)(float volume);

    int32_t (*load_sound)(libqu_file *file);
    void (*delete_sound)(int32_t sound_id);
    int32_t (*play_sound)(int32_t sound_id);
    int32_t (*loop_sound)(int32_t sound_id);

    int32_t (*open_music)(libqu_file *file);
    void (*close_music)(int32_t music_id);
    int32_t (*play_music)(int32_t music_id);
    int32_t (*loop_music)(int32_t music_id);

    void (*pause_stream)(int32_t stream_id);
    void (*unpause_stream)(int32_t stream_id);
    void (*stop_stream)(int32_t stream_id);
} libqu_audio;

void libqu_construct_null_audio(libqu_audio *audio);
void libqu_construct_openal_audio(libqu_audio *audio);

//------------------------------------------------------------------------------
// Gateway

bool libqu_gl_check_extension(char const *name);
void *libqu_gl_proc_address(char const *name);
void libqu_notify_gc_created(libqu_gc gc);
void libqu_notify_gc_destroyed(void);
void libqu_notify_display_resize(int width, int height);

//------------------------------------------------------------------------------

#endif // QU_PRIVATE_H_INC
