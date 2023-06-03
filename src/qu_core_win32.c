//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include "qu_core.h"
#include "qu_log.h"

//------------------------------------------------------------------------------

static void initialize(qu_params const *params)
{
    libqu_info("Null core module initialized.\n");
}

static void terminate(void)
{
    libqu_info("Null core module terminated.\n");
}

static bool is_initialized(void)
{
    return true;
}

static bool process(void)
{
    return false;
}

static void present(void)
{
}

static libqu_gc get_gc(void)
{
    return LIBQU_GC_NONE;
}

static bool gl_check_extension(char const *name)
{
    return false;
}

static void *gl_proc_address(char const *name)
{
    return NULL;
}

//------------------------------------------------------------------------------

static bool const *get_keyboard_state(void)
{
    static bool array[QU_TOTAL_KEYS] = { 0 };
    return array;
}

static bool is_key_pressed(qu_key key)
{
    return false;
}

//------------------------------------------------------------------------------

static uint8_t get_mouse_button_state(void)
{
    return 0;
}

static bool is_mouse_button_pressed(qu_mouse_button button)
{
    return false;
}

static qu_vec2i get_mouse_cursor_position(void)
{
    return (qu_vec2i) { 0, 0 };
}

static qu_vec2i get_mouse_cursor_delta(void)
{
    return (qu_vec2i) { 0, 0 };
}

static qu_vec2i get_mouse_wheel_delta(void)
{
    return (qu_vec2i) { 0, 0 };
}

//------------------------------------------------------------------------------

static bool is_joystick_connected(int joystick)
{
    return false;
}

static char const *get_joystick_id(int joystick)
{
    return NULL;
}

static int get_joystick_button_count(int joystick)
{
    return 0;
}

static int get_joystick_axis_count(int joystick)
{
    return 0;
}

static char const *get_joystick_button_id(int joystick, int button)
{
    return NULL;
}

static char const *get_joystick_axis_id(int joystick, int axis)
{
    return NULL;
}

static bool is_joystick_button_pressed(int joystick, int button)
{
    return false;
}

static float get_joystick_axis_value(int joystick, int axis)
{
    return 0.f;
}

//------------------------------------------------------------------------------

static void on_key_pressed(qu_key_fn fn)
{
}

static void on_key_repeated(qu_key_fn fn)
{
}

static void on_key_released(qu_key_fn fn)
{
}

static void on_mouse_button_pressed(qu_mouse_button_fn fn)
{
}

static void on_mouse_button_released(qu_mouse_button_fn fn)
{
}

static void on_mouse_cursor_moved(qu_mouse_cursor_fn fn)
{
}

static void on_mouse_wheel_scrolled(qu_mouse_wheel_fn fn)
{
}

//------------------------------------------------------------------------------

static float get_time_mediump(void)
{
    return 0.f;
}

static double get_time_highp(void)
{
    return 0.0;
}

//------------------------------------------------------------------------------

void libqu_construct_win32_core(libqu_core *core)
{
    *core = (libqu_core) {
        .initialize = initialize,
        .terminate = terminate,
        .is_initialized = is_initialized,
        .process = process,
        .present = present,
        .get_gc = get_gc,
        .gl_check_extension = gl_check_extension,
        .gl_proc_address = gl_proc_address,
        .get_keyboard_state = get_keyboard_state,
        .is_key_pressed = is_key_pressed,
        .get_mouse_button_state = get_mouse_button_state,
        .is_mouse_button_pressed = is_mouse_button_pressed,
        .get_mouse_cursor_position = get_mouse_cursor_position,
        .get_mouse_cursor_delta = get_mouse_cursor_delta,
        .get_mouse_wheel_delta = get_mouse_wheel_delta,
        .is_joystick_connected = is_joystick_connected,
        .get_joystick_id = get_joystick_id,
        .get_joystick_button_count = get_joystick_button_count,
        .get_joystick_axis_count = get_joystick_axis_count,
        .is_joystick_button_pressed = is_joystick_button_pressed,
        .get_joystick_axis_value = get_joystick_axis_value,
        .get_joystick_button_id = get_joystick_button_id,
        .get_joystick_axis_id = get_joystick_axis_id,
        .on_key_pressed = on_key_pressed,
        .on_key_repeated = on_key_repeated,
        .on_key_released = on_key_released,
        .on_mouse_button_pressed = on_mouse_button_pressed,
        .on_mouse_button_released = on_mouse_button_released,
        .on_mouse_cursor_moved = on_mouse_cursor_moved,
        .on_mouse_wheel_scrolled = on_mouse_wheel_scrolled,
        .get_time_mediump = get_time_mediump,
        .get_time_highp = get_time_highp,
    };
}

//------------------------------------------------------------------------------
