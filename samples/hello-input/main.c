//------------------------------------------------------------------------------

#include <stdio.h>
#include <libqu.h>

//------------------------------------------------------------------------------

#define FLAG_KEY_PRESSED            0x01
#define FLAG_KEY_REPEATED           0x02
#define FLAG_KEY_RELEASED           0x04
#define FLAG_BUTTON_PRESSED         0x08
#define FLAG_BUTTON_RELEASED        0x10
#define FLAG_CURSOR_MOVED           0x20
#define FLAG_WHEEL_SCROLLED         0x40

//------------------------------------------------------------------------------

static char const *key_names[QU_TOTAL_KEYS] = {
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "GRAVE",
    "APOSTROPHE",
    "MINUS",
    "EQUAL",
    "LBRACKET",
    "RBRACKET",
    "COMMA",
    "PERIOD",
    "SEMICOLON",
    "SLASH",
    "BACKSLASH",
    "SPACE",
    "ESCAPE",
    "BACKSPACE",
    "TAB",
    "ENTER",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "LSHIFT",
    "RSHIFT",
    "LCTRL",
    "RCTRL",
    "LALT",
    "RALT",
    "LSUPER",
    "RSUPER",
    "MENU",
    "PGUP",
    "PGDN",
    "HOME",
    "END",
    "INSERT",
    "DELETE",
    "PRINTSCREEN",
    "PAUSE",
    "CAPSLOCK",
    "SCROLLLOCK",
    "NUMLOCK",
    "KP_0",
    "KP_1",
    "KP_2",
    "KP_3",
    "KP_4",
    "KP_5",
    "KP_6",
    "KP_7",
    "KP_8",
    "KP_9",
    "KP_MUL",
    "KP_ADD",
    "KP_SUB",
    "KP_POINT",
    "KP_DIV",
    "KP_ENTER",
    "[INVALID]",
};

static char const *button_names[QU_TOTAL_MOUSE_BUTTONS] = {
    "LEFT",
    "RIGHT",
    "MIDDLE",
    "[INVALID]",
};

//------------------------------------------------------------------------------

static qu_font font;

static struct
{
    int flags;
    qu_key last_key_press;
    qu_key last_key_repeat;
    qu_key last_key_release;
    qu_mouse_button last_button_press;
    qu_mouse_button last_button_release;
    qu_vec2i last_cursor_motion;
    qu_vec2i last_wheel_scroll;
} state;

static void on_key_pressed(qu_key key)
{
    state.flags |= FLAG_KEY_PRESSED;
    state.last_key_press = key;
}

static void on_key_repeated(qu_key key)
{
    state.flags |= FLAG_KEY_REPEATED;
    state.last_key_repeat = key;
}

static void on_key_released(qu_key key)
{
    state.flags |= FLAG_KEY_RELEASED;
    state.last_key_release = key;
}

static void on_mouse_button_pressed(qu_mouse_button button)
{
    state.flags |= FLAG_BUTTON_PRESSED;
    state.last_button_press = button;
}

static void on_mouse_button_released(qu_mouse_button button)
{
    state.flags |= FLAG_BUTTON_RELEASED;
    state.last_button_release = button;
}

static void on_mouse_cursor_moved(int x_delta, int y_delta)
{
    state.flags |= FLAG_CURSOR_MOVED;
    state.last_cursor_motion.x = x_delta;
    state.last_cursor_motion.y = y_delta;
}

static void on_mouse_wheel_scrolled(int x_delta, int y_delta)
{
    state.flags |= FLAG_WHEEL_SCROLLED;
    state.last_wheel_scroll.x = x_delta;
    state.last_wheel_scroll.y = y_delta;
}

static bool loop(void)
{
    qu_clear(0xff1c1c1c);

    float x = 16.f;
    float y = 16.f;
    float n = 0;

    // *** Keyboard ***

    qu_draw_text(font, x, y + 20.f * (n++), 0xffefef00, "*** Keyboard ***");

    if (qu_is_key_pressed(QU_KEY_SPACE)) {
        qu_draw_text(font, x, y + 20.f * (n++), 0xffff0000,
            "qu_is_key_pressed: press SPACE to test");
    } else {
        qu_draw_text(font, x, y + 20.f * (n++), 0xffe0e0e0,
            "qu_is_key_pressed: press SPACE to test");
    }

    bool const *kbd = qu_get_keyboard_state();
    int key_count = 0;
    qu_draw_text(font, x, y + 20.f * (n++), 0xffe0e0e0,
        "qu_get_keyboard_state:");

    for (int i = 0; i < QU_TOTAL_KEYS; i++) {
        if (kbd[i]) {
            qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xff56789a, "0x%02x: %s", i, key_names[i]);
            key_count++;
        }
    }

    if (key_count == 0) {
        qu_draw_text(font, x, y + 20.f * (n++), 0xff56789a, "(no keys pressed)");
    }

    // *** Mouse ***

    n++;
    qu_draw_text(font, x, y + 20.f * (n++), 0xffefef00, "*** Mouse ***");

    uint8_t button_state = qu_get_mouse_button_state();
    qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xffe0e0e0,
        "qu_get_mouse_button_state: 0x%02x", button_state);

    if (qu_is_mouse_button_pressed(QU_MOUSE_BUTTON_LEFT)) {
        qu_draw_text(font, x, y + 20.f * (n++), 0xffff0000,
            "qu_is_mouse_button_pressed: press LEFT button to test");
    } else {
        qu_draw_text(font, x, y + 20.f * (n++), 0xffe0e0e0,
            "qu_is_mouse_button_pressed: press LEFT button to test");
    }

    qu_vec2i wheel_delta = qu_get_mouse_wheel_delta();
    qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xffe0e0e0,
        "qu_get_mouse_wheel_delta: [%d, %d]",
        wheel_delta.x, wheel_delta.y);

    qu_vec2i cursor_pos = qu_get_mouse_cursor_position();
    qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xffe0e0e0,
        "qu_get_mouse_cursor_position: [%d, %d]",
        cursor_pos.x, cursor_pos.y);

    qu_vec2i cursor_delta = qu_get_mouse_cursor_delta();
    qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xffe0e0e0,
        "qu_get_mouse_cursor_delta: [%d, %d]",
        cursor_delta.x, cursor_delta.y);

    // *** Joystick ***

    n++;
    qu_draw_text(font, x, y + 20.f * (n++), 0xffefef00, "*** Joystick ***");

    if (qu_is_joystick_connected(0)) {
        qu_draw_text(font, x, y + 20.f * (n++), 0xffe0e0e0,
            "qu_is_joystick_connected: #0 is connected");
        
        int button_count = qu_get_joystick_button_count(0);
        int axis_count = qu_get_joystick_axis_count(0);

        qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xffe0e0e0,
            "qu_get_joystick_button_count: %d", button_count);
        
        qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xffe0e0e0,
            "qu_get_joystick_axis_count: %d", axis_count);
        
        qu_draw_text(font, x, y + 20.f * (n++), 0xffe0e0e0,
            "qu_get_joystick_button_id, qu_is_joystick_button_pressed:");

        for (int i = 0; i < button_count; i++) {
            qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xff56789a,
                "[%s]: %d", qu_get_joystick_button_id(0, i), qu_is_joystick_button_pressed(0, i));
        }

        qu_draw_text(font, x, y + 20.f * (n++), 0xffe0e0e0,
            "qu_get_joystick_axis_id, qu_get_joystick_axis_value:");

        for (int i = 0; i < axis_count; i++) {
            qu_draw_text_fmt(font, x, y + 20.f * (n++), 0xff56789a,
                "[%s]: %.2f", qu_get_joystick_axis_id(0, i), qu_get_joystick_axis_value(0, i));
        }
    } else {
        qu_draw_text(font, x, y + 20.f * (n++), 0xff56789a,
            "qu_is_joystick_connected: #0 is disconnected");
    }

    // *** Event callbacks ***

    x = 512.f;
    n = 0;

    qu_draw_text(font, x, y + 20.f * (n++), 0xffefef00, "*** Event callbacks ***");

    qu_draw_text_fmt(font, x, y + 20.f * (n++),
        (state.flags & FLAG_KEY_PRESSED) ? 0xffff0000 : 0xffe0e0e0,
        "qu_on_key_pressed: last pressed key is %s",
        key_names[state.last_key_press]);

    qu_draw_text_fmt(font, x, y + 20.f * (n++),
        (state.flags & FLAG_KEY_REPEATED) ? 0xffff0000 : 0xffe0e0e0,
        "qu_on_key_repeated: last repeated key is %s",
        key_names[state.last_key_repeat]);

    qu_draw_text_fmt(font, x, y + 20.f * (n++),
        (state.flags & FLAG_KEY_RELEASED) ? 0xffff0000 : 0xffe0e0e0,
        "qu_on_key_released: last released key is %s",
        key_names[state.last_key_release]);

    qu_draw_text_fmt(font, x, y + 20.f * (n++),
        (state.flags & FLAG_BUTTON_PRESSED) ? 0xffff0000 : 0xffe0e0e0,
        "qu_on_mouse_button_pressed: last pressed button is %s",
        button_names[state.last_button_press]);

    qu_draw_text_fmt(font, x, y + 20.f * (n++),
        (state.flags & FLAG_BUTTON_RELEASED) ? 0xffff0000 : 0xffe0e0e0,
        "qu_on_mouse_button_released: last pressed button is %s",
        button_names[state.last_button_release]);

    qu_draw_text_fmt(font, x, y + 20.f * (n++),
        (state.flags & FLAG_CURSOR_MOVED) ? 0xffff0000 : 0xffe0e0e0,
        "qu_on_mouse_cursor_moved: last delta is [%d, %d]",
        state.last_cursor_motion.x, state.last_cursor_motion.y);

    qu_draw_text_fmt(font, x, y + 20.f * (n++),
        (state.flags & FLAG_WHEEL_SCROLLED) ? 0xffff0000 : 0xffe0e0e0,
        "qu_on_mouse_wheel_scrolled: last delta is [%d, %d]",
        state.last_wheel_scroll.x, state.last_wheel_scroll.y);

    qu_present();

    state.flags = 0;

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "[libqu] hello-input",
        .display_width = 1024,
        .display_height = 512,
    });

    font = qu_load_font("assets/font.ttf", 18);

    if (!font.id) {
        qu_terminate();
        return 0;
    }

    qu_on_key_pressed(on_key_pressed);
    qu_on_key_repeated(on_key_repeated);
    qu_on_key_released(on_key_released);
    qu_on_mouse_button_pressed(on_mouse_button_pressed);
    qu_on_mouse_button_released(on_mouse_button_released);
    qu_on_mouse_cursor_moved(on_mouse_cursor_moved);
    qu_on_mouse_wheel_scrolled(on_mouse_wheel_scrolled);

    state.last_key_press = QU_KEY_INVALID;
    state.last_key_repeat = QU_KEY_INVALID;
    state.last_key_release = QU_KEY_INVALID;
    state.last_button_press = QU_MOUSE_BUTTON_INVALID;
    state.last_button_release = QU_MOUSE_BUTTON_INVALID;

    qu_execute(loop);

    return 0;
}

//------------------------------------------------------------------------------
