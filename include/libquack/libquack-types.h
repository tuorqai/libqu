//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef LIBQUACK_TYPES_H
#define LIBQUACK_TYPES_H

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

typedef enum qu_key qu_key;
typedef enum qu_mouse_button qu_mouse_button;
typedef enum qu_screen_mode qu_screen_mode;

//------------------------------------------------------------------------------

typedef uint64_t qu_color;

//------------------------------------------------------------------------------

typedef struct qu_vec2f
{
    float x;
    float y;
} qu_vec2f;

typedef struct qu_vec2i
{
    int x;
    int y;
} qu_vec2i;

//------------------------------------------------------------------------------

typedef struct qu_texture
{
    int32_t id;
} qu_texture;

typedef struct qu_surface
{
    int32_t id;
} qu_surface;

typedef struct qu_font
{
    int32_t id;
} qu_font;

typedef struct qu_sound
{
    int32_t id;
} qu_sound;

typedef struct qu_music
{
    int32_t id;
} qu_music;

typedef struct qu_stream
{
    int32_t id;
} qu_stream;

//------------------------------------------------------------------------------

typedef struct qu_params
{
    char const *title;
    int display_width;
    int display_height;
    qu_screen_mode screen_mode;
} qu_params;

//------------------------------------------------------------------------------

typedef bool (*qu_loop_fn)(void);
typedef void (*qu_key_fn)(qu_key key);
typedef void (*qu_mouse_button_fn)(qu_mouse_button button);
typedef void (*qu_mouse_wheel_fn)(int x_delta, int y_delta);
typedef void (*qu_mouse_cursor_fn)(int x_delta, int y_delta);

//------------------------------------------------------------------------------

#endif // LIBQUACK_TYPES_H

//------------------------------------------------------------------------------
