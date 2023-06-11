//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef LIBQU_H
#define LIBQU_H

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

//------------------------------------------------------------------------------

#if defined(QU_SHARED)
#   if defined(_MSC_VER)
#       if defined(QU_BUILD)
#           define QU_API __declspec(dllexport)
#       else
#           define QU_API __declspec(dllimport)
#       endif
#   elif defined(__GNUC__)
#       define QU_API __attribute__((visibility("default")))
#   else
#       define QU_API
#   endif
#else
#   define QU_API
#endif

#if defined(_WIN32)
#   define QU_CALL __cdecl
#else
#   define QU_CALL
#endif

#if defined(_MSC_VER)
#   define QU_NO_RET __declspec(noreturn)
#elif defined(__GNUC__)
#   define QU_NO_RET __attribute__((__noreturn__))
#else
#   define QU_NO_RET
#endif

//------------------------------------------------------------------------------

#define QU_PI               3.14159265358979323846
#define QU_DEG2RAD(Deg)     ((Deg) * (QU_PI / 180.0))
#define QU_RAD2DEG(Rad)     ((Rad) * (180.0 / QU_PI))
#define QU_MAX(A, B)        ((A) > (B) ? (A) : (B))
#define QU_MIN(A, B)        ((A) < (B) ? (A) : (B))

#define QU_COLOR(Red, Green, Blue) \
    (255 | (Red) << 16 | (Green) << 8 | (Blue))

#define QU_RGBA(Red, Green, Blue, Alpha) \
    ((Alpha) << 24 | (Red) << 16 | (Green) << 8 | (Blue))

//------------------------------------------------------------------------------

typedef enum qu_key
{
    QU_KEY_0,
    QU_KEY_1,
    QU_KEY_2,
    QU_KEY_3,
    QU_KEY_4,
    QU_KEY_5,
    QU_KEY_6,
    QU_KEY_7,
    QU_KEY_8,
    QU_KEY_9,
    QU_KEY_A,
    QU_KEY_B,
    QU_KEY_C,
    QU_KEY_D,
    QU_KEY_E,
    QU_KEY_F,
    QU_KEY_G,
    QU_KEY_H,
    QU_KEY_I,
    QU_KEY_J,
    QU_KEY_K,
    QU_KEY_L,
    QU_KEY_M,
    QU_KEY_N,
    QU_KEY_O,
    QU_KEY_P,
    QU_KEY_Q,
    QU_KEY_R,
    QU_KEY_S,
    QU_KEY_T,
    QU_KEY_U,
    QU_KEY_W,
    QU_KEY_X,
    QU_KEY_Y,
    QU_KEY_Z,
    QU_KEY_GRAVE,
    QU_KEY_APOSTROPHE,
    QU_KEY_MINUS,
    QU_KEY_EQUAL,
    QU_KEY_LBRACKET,
    QU_KEY_RBRACKET,
    QU_KEY_COMMA,
    QU_KEY_PERIOD,
    QU_KEY_SEMICOLON,
    QU_KEY_SLASH,
    QU_KEY_BACKSLASH,
    QU_KEY_SPACE,
    QU_KEY_ESCAPE,
    QU_KEY_BACKSPACE,
    QU_KEY_TAB,
    QU_KEY_ENTER,
    QU_KEY_F1,
    QU_KEY_F2,
    QU_KEY_F3,
    QU_KEY_F4,
    QU_KEY_F5,
    QU_KEY_F6,
    QU_KEY_F7,
    QU_KEY_F8,
    QU_KEY_F9,
    QU_KEY_F10,
    QU_KEY_F11,
    QU_KEY_F12,
    QU_KEY_UP,
    QU_KEY_DOWN,
    QU_KEY_LEFT,
    QU_KEY_RIGHT,
    QU_KEY_LSHIFT,
    QU_KEY_RSHIFT,
    QU_KEY_LCTRL,
    QU_KEY_RCTRL,
    QU_KEY_LALT,
    QU_KEY_RALT,
    QU_KEY_LSUPER,
    QU_KEY_RSUPER,
    QU_KEY_MENU,
    QU_KEY_PGUP,
    QU_KEY_PGDN,
    QU_KEY_HOME,
    QU_KEY_END,
    QU_KEY_INSERT,
    QU_KEY_DELETE,
    QU_KEY_PRINTSCREEN,
    QU_KEY_PAUSE,
    QU_KEY_CAPSLOCK,
    QU_KEY_SCROLLLOCK,
    QU_KEY_NUMLOCK,
    QU_KEY_KP_0,
    QU_KEY_KP_1,
    QU_KEY_KP_2,
    QU_KEY_KP_3,
    QU_KEY_KP_4,
    QU_KEY_KP_5,
    QU_KEY_KP_6,
    QU_KEY_KP_7,
    QU_KEY_KP_8,
    QU_KEY_KP_9,
    QU_KEY_KP_MUL,
    QU_KEY_KP_ADD,
    QU_KEY_KP_SUB,
    QU_KEY_KP_POINT,
    QU_KEY_KP_DIV,
    QU_KEY_KP_ENTER,
    QU_KEY_INVALID,
    QU_TOTAL_KEYS,
} qu_key;

typedef enum qu_mouse_button
{
    QU_MOUSE_BUTTON_LEFT,
    QU_MOUSE_BUTTON_RIGHT,
    QU_MOUSE_BUTTON_MIDDLE,
    QU_MOUSE_BUTTON_INVALID,
    QU_TOTAL_MOUSE_BUTTONS,
} qu_mouse_button;

typedef enum qu_mouse_button_bits
{
    QU_MOUSE_BUTTON_LEFT_BIT = (1 << QU_MOUSE_BUTTON_LEFT),
    QU_MOUSE_BUTTON_RIGHT_BIT = (1 << QU_MOUSE_BUTTON_RIGHT),
    QU_MOUSE_BUTTON_MIDDLE_BIT = (1 << QU_MOUSE_BUTTON_MIDDLE),
} qu_mouse_button_bits;

typedef enum qu_screen_mode
{
    QU_SCREEN_MODE_DEFAULT,
    QU_SCREEN_MODE_UPDATE_VIEW,
    QU_SCREEN_MODE_USE_CANVAS,
} qu_screen_mode;

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

#if defined(__cplusplus)
extern "C" {
#endif

//------------------------------------------------------------------------------

// Initialize the library. One can use NULL for default parameters.
QU_API void QU_CALL qu_initialize(qu_params const *params);

// Terminate the library.
QU_API void QU_CALL qu_terminate(void);

// Process user input. Returns false if the user requested exit.
// It's recommended to call this every frame.
QU_API bool QU_CALL qu_process(void);

// Enter main loop.
QU_API QU_NO_RET void QU_CALL qu_execute(qu_loop_fn loop_fn);

// Draw everything on screen.
QU_API void QU_CALL qu_present(void);

//------------------------------------------------------------------------------

// Get current keyboard state as an array of QU_TOTAL_KEYS bools.
QU_API bool const * QU_CALL qu_get_keyboard_state(void);

// Check if a key is pressed.
QU_API bool QU_CALL qu_is_key_pressed(qu_key key);

// Get current mouse button state as a bitmask.
QU_API uint8_t QU_CALL qu_get_mouse_button_state(void);

// Check if a mouse button is pressed.
QU_API bool QU_CALL qu_is_mouse_button_pressed(qu_mouse_button button);

// Get current mouse cursor position relative to the window.
QU_API qu_vec2i QU_CALL qu_get_mouse_cursor_position(void);

// Get mouse cursor movement amout since the last frame.
QU_API qu_vec2i QU_CALL qu_get_mouse_cursor_delta(void);

// Get mouse wheel movement amount since the last frame.
QU_API qu_vec2i QU_CALL qu_get_mouse_wheel_delta(void);

// Get how many joysticks are connected.
QU_API bool QU_CALL qu_is_joystick_connected(int);

// Get joystick identifier.
QU_API char const * QU_CALL qu_get_joystick_id(int joystick);

// Get how many buttons joystick does have.
QU_API int QU_CALL qu_get_joystick_button_count(int joystick);

// Get how many axes joystick does have.
QU_API int QU_CALL qu_get_joystick_axis_count(int joystick);

// Get the identifier of a joystick button.
QU_API char const * QU_CALL qu_get_joystick_button_id(int joystick, int button);

// Get the identifier of a joystick axis.
QU_API char const * QU_CALL qu_get_joystick_axis_id(int joystick, int axis);

// Check if a button is pressed.
QU_API bool QU_CALL qu_is_joystick_button_pressed(int joystick, int button);

// Get joystick axis value.
QU_API float QU_CALL qu_get_joystick_axis_value(int joystick, int axis);

// Set callback function that will be called when a key is pressed.
QU_API void QU_CALL qu_on_key_pressed(qu_key_fn fn);

// Set callback function that will be called when a key is hold down.
QU_API void QU_CALL qu_on_key_repeated(qu_key_fn fn);

// Set callback function that will be called when a key is released.
QU_API void QU_CALL qu_on_key_released(qu_key_fn fn);

// Set callback function that will be called when a mouse button is pressed.
QU_API void QU_CALL qu_on_mouse_button_pressed(qu_mouse_button_fn fn);

// Set callback function that will be called when a mouse button is released.
QU_API void QU_CALL qu_on_mouse_button_released(qu_mouse_button_fn fn);

// Set callback function that will be called when a mouse cursor is moved.
QU_API void QU_CALL qu_on_mouse_cursor_moved(qu_mouse_cursor_fn fn);

// Set callback function that will be called when a mouse wheel is scrolled.
QU_API void QU_CALL qu_on_mouse_wheel_scrolled(qu_mouse_wheel_fn fn);

// Get amount of time passed since the initialization. Medium precision.
QU_API float QU_CALL qu_get_time_mediump(void);

// Get amount of time passed since the initialization. High precision.
QU_API double QU_CALL qu_get_time_highp(void);

//------------------------------------------------------------------------------

QU_API void QU_CALL qu_set_view(float x, float y, float w, float h, float rotation);
QU_API void QU_CALL qu_reset_view(void);

// Clear the screen.
QU_API void QU_CALL qu_clear(qu_color color);

// Draw point.
QU_API void QU_CALL qu_draw_point(float x, float y, qu_color color);

// Draw line.
QU_API void QU_CALL qu_draw_line(float ax, float ay, float bx, float by,
                                 qu_color color);

// Draw triangle.
QU_API void QU_CALL qu_draw_triangle(float ax, float ay, float bx, float by,
                                     float cx, float cy, qu_color outline,
                                     qu_color fill);

// Draw rectangle.
QU_API void QU_CALL qu_draw_rectangle(float x, float y, float w, float h,
                                      qu_color outline, qu_color fill);

// Draw circle.
QU_API void QU_CALL qu_draw_circle(float x, float y, float radius,
                                   qu_color outline, qu_color fill);

QU_API qu_texture QU_CALL qu_load_texture(char const *path);
QU_API void QU_CALL qu_delete_texture(qu_texture texture);
QU_API void QU_CALL qu_set_texture_smooth(qu_texture texture, bool smooth);
QU_API void QU_CALL qu_draw_texture(qu_texture texture, float x, float y, float w, float h);
QU_API void QU_CALL qu_draw_subtexture(qu_texture texture, float x, float y, float w, float h, float rx, float ry, float rw, float rh);

QU_API qu_font QU_CALL qu_load_font(char const *path, float pt);
QU_API void QU_CALL qu_delete_font(qu_font font);
QU_API void QU_CALL qu_draw_text(qu_font font, float x, float y, qu_color color, char const *str);
QU_API void QU_CALL qu_draw_text_fmt(qu_font font, float x, float y, qu_color color, char const *fmt, ...);

QU_API qu_surface QU_CALL qu_create_surface(int width, int height);
QU_API void QU_CALL qu_delete_surface(qu_surface surface);
QU_API void QU_CALL qu_set_surface(qu_surface surface);
QU_API void QU_CALL qu_reset_surface(void);
QU_API void QU_CALL qu_draw_surface(qu_surface surface, float x, float y, float w, float h);

//------------------------------------------------------------------------------

/**
 * Set master volume.
 *
 * \param volume Volume level from 0.0 to 1.0
 */
QU_API void QU_CALL qu_set_master_volume(float volume);

// Load sound file to memory.
QU_API qu_sound QU_CALL qu_load_sound(char const *path);

// Delete sound from memory.
QU_API void QU_CALL qu_delete_sound(qu_sound sound);

// Play sound and get sound stream.
QU_API qu_stream QU_CALL qu_play_sound(qu_sound sound);

// Loop sound and get sound stream.
QU_API qu_stream QU_CALL qu_loop_sound(qu_sound sound);

QU_API qu_music QU_CALL qu_open_music(char const *path);
QU_API void QU_CALL qu_close_music(qu_music music);
QU_API qu_stream QU_CALL qu_play_music(qu_music music);
QU_API qu_stream QU_CALL qu_loop_music(qu_music music);

// Pause sound stream.
QU_API void QU_CALL qu_pause_stream(qu_stream stream);

// Unpause sound stream.
QU_API void QU_CALL qu_unpause_stream(qu_stream stream);

// Stop sound stream. The stream is lost after that.
QU_API void QU_CALL qu_stop_stream(qu_stream stream);

//------------------------------------------------------------------------------

#if defined(__cplusplus)
} // extern "C"
#endif

//------------------------------------------------------------------------------

#endif // LIBQU_H

//------------------------------------------------------------------------------
