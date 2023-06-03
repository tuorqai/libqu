//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_CORE_H
#define QU_CORE_H

//------------------------------------------------------------------------------

#include "libqu.h"

//------------------------------------------------------------------------------

typedef enum libqu_gc
{
    LIBQU_GC_GL,
    LIBQU_GC_GLES,
    LIBQU_GC_NONE,
} libqu_gc;

typedef struct libqu_core
{
    void (*initialize)(qu_params const *params);
    void (*terminate)(void);
    bool (*is_initialized)(void);
    bool (*process)(void);
    void (*present)(void);
    libqu_gc (*get_gc)(void);
    bool (*gl_check_extension)(char const *name);
    void *(*gl_proc_address)(char const *name);

    bool const *(*get_keyboard_state)(void);
    bool (*is_key_pressed)(qu_key key);

    uint8_t (*get_mouse_button_state)(void);
    bool (*is_mouse_button_pressed)(qu_mouse_button button);
    qu_vec2i (*get_mouse_cursor_position)(void);
    qu_vec2i (*get_mouse_cursor_delta)(void);
    qu_vec2i (*get_mouse_wheel_delta)(void);

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

//------------------------------------------------------------------------------

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

#endif // QU_CORE_H

//------------------------------------------------------------------------------