//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#if defined(__EMSCRIPTEN__)

//------------------------------------------------------------------------------

#include "qu.h"
#include <SDL.h>

//------------------------------------------------------------------------------

static struct
{
    bool initialized;
    SDL_Surface *display;

    struct {
        bool keyboard[256];
        uint8_t mouse;
        int x_mouse, y_mouse;
        int dx_mouse, dy_mouse;
        int dx_wheel, dy_wheel;
    } input;

    struct {
        qu_key_fn on_key_pressed;
        qu_key_fn on_key_repeated;
        qu_key_fn on_key_released;
        qu_mouse_button_fn on_mouse_button_pressed;
        qu_mouse_button_fn on_mouse_button_released;
        qu_mouse_wheel_fn on_mouse_wheel_scrolled;
        qu_mouse_cursor_fn on_mouse_cursor_moved;
    } callbacks;
} impl;

//------------------------------------------------------------------------------

static qu_key key_conv(SDL_Keysym *sym)
{
    switch (sym->sym) {
    case SDLK_0:                return QU_KEY_0;
    case SDLK_1:                return QU_KEY_1;
    case SDLK_2:                return QU_KEY_2;
    case SDLK_3:                return QU_KEY_3;
    case SDLK_4:                return QU_KEY_4;
    case SDLK_5:                return QU_KEY_5;
    case SDLK_6:                return QU_KEY_6;
    case SDLK_7:                return QU_KEY_7;
    case SDLK_8:                return QU_KEY_8;
    case SDLK_9:                return QU_KEY_9;
    case SDLK_a:                return QU_KEY_A;
    case SDLK_b:                return QU_KEY_B;
    case SDLK_c:                return QU_KEY_C;
    case SDLK_d:                return QU_KEY_D;
    case SDLK_e:                return QU_KEY_E;
    case SDLK_f:                return QU_KEY_F;
    case SDLK_g:                return QU_KEY_G;
    case SDLK_h:                return QU_KEY_H;
    case SDLK_i:                return QU_KEY_I;
    case SDLK_j:                return QU_KEY_J;
    case SDLK_k:                return QU_KEY_K;
    case SDLK_l:                return QU_KEY_L;
    case SDLK_m:                return QU_KEY_M;
    case SDLK_n:                return QU_KEY_N;
    case SDLK_o:                return QU_KEY_O;
    case SDLK_p:                return QU_KEY_P;
    case SDLK_q:                return QU_KEY_Q;
    case SDLK_r:                return QU_KEY_R;
    case SDLK_s:                return QU_KEY_S;
    case SDLK_t:                return QU_KEY_T;
    case SDLK_u:                return QU_KEY_U;
    case SDLK_w:                return QU_KEY_W;
    case SDLK_x:                return QU_KEY_X;
    case SDLK_y:                return QU_KEY_Y;
    case SDLK_z:                return QU_KEY_Z;
    case SDLK_BACKQUOTE:        return QU_KEY_GRAVE;
    case SDLK_QUOTE:            return QU_KEY_APOSTROPHE;
    case SDLK_MINUS:            return QU_KEY_MINUS;
    case SDLK_EQUALS:           return QU_KEY_EQUAL;
    case SDLK_LEFTBRACKET:      return QU_KEY_LBRACKET;
    case SDLK_RIGHTBRACKET:     return QU_KEY_RBRACKET;
    case SDLK_COMMA:            return QU_KEY_COMMA;
    case SDLK_PERIOD:           return QU_KEY_PERIOD;
    case SDLK_SEMICOLON:        return QU_KEY_SEMICOLON;
    case SDLK_SLASH:            return QU_KEY_SLASH;
    case SDLK_BACKSLASH:        return QU_KEY_BACKSLASH;
    case SDLK_SPACE:            return QU_KEY_SPACE;
    case SDLK_ESCAPE:           return QU_KEY_ESCAPE;
    case SDLK_BACKSPACE:        return QU_KEY_BACKSPACE;
    case SDLK_TAB:              return QU_KEY_TAB;
    case SDLK_RETURN:           return QU_KEY_ENTER;
    case SDLK_F1:               return QU_KEY_F1;
    case SDLK_F2:               return QU_KEY_F2;
    case SDLK_F3:               return QU_KEY_F3;
    case SDLK_F4:               return QU_KEY_F4;
    case SDLK_F5:               return QU_KEY_F5;
    case SDLK_F6:               return QU_KEY_F6;
    case SDLK_F7:               return QU_KEY_F7;
    case SDLK_F8:               return QU_KEY_F8;
    case SDLK_F9:               return QU_KEY_F9;
    case SDLK_F10:              return QU_KEY_F10;
    case SDLK_F11:              return QU_KEY_F11;
    case SDLK_F12:              return QU_KEY_F12;
    case SDLK_UP:               return QU_KEY_UP;
    case SDLK_DOWN:             return QU_KEY_DOWN;
    case SDLK_LEFT:             return QU_KEY_LEFT;
    case SDLK_RIGHT:            return QU_KEY_RIGHT;
    case SDLK_LSHIFT:           return QU_KEY_LSHIFT;
    case SDLK_RSHIFT:           return QU_KEY_RSHIFT;
    case SDLK_LCTRL:            return QU_KEY_LCTRL;
    case SDLK_RCTRL:            return QU_KEY_RCTRL;
    case SDLK_LALT:             return QU_KEY_LALT;
    case SDLK_RALT:             return QU_KEY_RALT;
    case SDLK_LSUPER:           return QU_KEY_LSUPER;
    case SDLK_RSUPER:           return QU_KEY_RSUPER;
    case SDLK_MENU:             return QU_KEY_MENU;
    case SDLK_PAGEUP:           return QU_KEY_PGUP;
    case SDLK_PAGEDOWN:         return QU_KEY_PGDN;
    case SDLK_HOME:             return QU_KEY_HOME;
    case SDLK_END:              return QU_KEY_END;
    case SDLK_INSERT:           return QU_KEY_INSERT;
    case SDLK_DELETE:           return QU_KEY_DELETE;
    case SDLK_PRINTSCREEN:      return QU_KEY_PRINTSCREEN;
    case SDLK_PAUSE:            return QU_KEY_PAUSE;
    case SDLK_CAPSLOCK:         return QU_KEY_CAPSLOCK;
    case SDLK_SCROLLLOCK:       return QU_KEY_SCROLLLOCK;
    case SDLK_NUMLOCK:          return QU_KEY_NUMLOCK;
    case SDLK_KP_0:             return QU_KEY_KP_0;
    case SDLK_KP_1:             return QU_KEY_KP_1;
    case SDLK_KP_2:             return QU_KEY_KP_2;
    case SDLK_KP_3:             return QU_KEY_KP_3;
    case SDLK_KP_4:             return QU_KEY_KP_4;
    case SDLK_KP_5:             return QU_KEY_KP_5;
    case SDLK_KP_6:             return QU_KEY_KP_6;
    case SDLK_KP_7:             return QU_KEY_KP_7;
    case SDLK_KP_8:             return QU_KEY_KP_8;
    case SDLK_KP_9:             return QU_KEY_KP_9;
    case SDLK_KP_MULTIPLY:      return QU_KEY_KP_MUL;
    case SDLK_KP_PLUS:          return QU_KEY_KP_ADD;
    case SDLK_KP_MINUS:         return QU_KEY_KP_SUB;
    case SDLK_KP_PERIOD:        return QU_KEY_KP_POINT;
    case SDLK_KP_DIVIDE:        return QU_KEY_KP_DIV;
    case SDLK_KP_ENTER:         return QU_KEY_KP_ENTER;
    default:                    return QU_KEY_INVALID;
    }
}

static qu_mouse_button mb_conv(Uint8 button)
{
    switch (button) {
        case SDL_BUTTON_LEFT:   return QU_MOUSE_BUTTON_LEFT;
        case SDL_BUTTON_MIDDLE: return QU_MOUSE_BUTTON_MIDDLE;
        case SDL_BUTTON_RIGHT:  return QU_MOUSE_BUTTON_RIGHT;
        default:
            break;
    }

    return QU_MOUSE_BUTTON_INVALID;
}

//------------------------------------------------------------------------------

static void initialize(qu_params const *params)
{
    memset(&impl, 0, sizeof(impl));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1) {
        libqu_error("Failed to initialize SDL.\n");
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    impl.display = SDL_SetVideoMode(
        params->display_width,
        params->display_height,
        32,
        SDL_OPENGL | SDL_RESIZABLE
    );

    if (!impl.display) {
        libqu_error("Failed to initialize display.\n");
        return;
    }

    SDL_WM_SetCaption(params->title, NULL);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    libqu_info("Emscripten core module initialized.\n");
    impl.initialized = true;
}

static void terminate(void)
{
    SDL_Quit();

    if (impl.initialized) {
        libqu_info("Emscripten core module terminated.\n");
        impl.initialized = false;
    }
}

static bool is_initialized(void)
{
    return impl.initialized;
}

static bool process(void)
{
    impl.input.dx_mouse = 0;
    impl.input.dy_mouse = 0;
    impl.input.dx_wheel = 0;
    impl.input.dy_wheel = 0;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
            case SDL_KEYDOWN: {
                qu_key key = key_conv(&event.key.keysym);

                if (key != QU_KEY_INVALID) {
                    if (impl.input.keyboard[key]) {
                        if (impl.callbacks.on_key_repeated) {
                            impl.callbacks.on_key_repeated(key);
                        }
                    } else {
                        impl.input.keyboard[key] = true;

                        if (impl.callbacks.on_key_pressed) {
                            impl.callbacks.on_key_pressed(key);
                        }
                    }
                }
                break;
            }
            case SDL_KEYUP: {
                qu_key key = key_conv(&event.key.keysym);

                if (key != QU_KEY_INVALID) {
                    impl.input.keyboard[key] = false;

                    if (impl.callbacks.on_key_released) {
                        impl.callbacks.on_key_released(key);
                    }
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                qu_mouse_button button = mb_conv(event.button.button);

                if (button != QU_MOUSE_BUTTON_INVALID) {
                    impl.input.mouse |= (1 << button);

                    if (impl.callbacks.on_mouse_button_pressed) {
                        impl.callbacks.on_mouse_button_pressed(button);
                    }
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                qu_mouse_button button = mb_conv(event.button.button);

                if (button != QU_MOUSE_BUTTON_INVALID) {
                    impl.input.mouse &= ~(1 << button);

                    if (impl.callbacks.on_mouse_button_released) {
                        impl.callbacks.on_mouse_button_released(button);
                    }
                }
                break;
            }
            case SDL_MOUSEMOTION: {
                impl.input.x_mouse = event.motion.x;
                impl.input.y_mouse = event.motion.y;
                impl.input.dx_mouse = event.motion.xrel;
                impl.input.dy_mouse = event.motion.yrel;

                if (impl.callbacks.on_mouse_cursor_moved) {
                    impl.callbacks.on_mouse_cursor_moved(
                        impl.input.dx_mouse, impl.input.dy_mouse
                    );
                }
                break;
            }
            case SDL_MOUSEWHEEL: {
                impl.input.dx_wheel = event.wheel.x;
                impl.input.dy_wheel = event.wheel.y;

                if (impl.callbacks.on_mouse_wheel_scrolled) {
                    impl.callbacks.on_mouse_wheel_scrolled(
                        impl.input.dx_wheel, impl.input.dy_wheel
                    );
                }
                break;
            }
            default:
                break;
        }
    }

    return true;
}

static void present(void)
{
    SDL_GL_SwapBuffers();
}

static libqu_gc get_gc(void)
{
    return LIBQU_GC_GLES;
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
    return impl.input.keyboard;
}

static bool is_key_pressed(qu_key key)
{
    return impl.input.keyboard[key];
}

//------------------------------------------------------------------------------

static uint8_t get_mouse_button_state(void)
{
    return impl.input.mouse;
}

static bool is_mouse_button_pressed(qu_mouse_button button)
{
    return impl.input.mouse & (1 << button);
}

static qu_vec2i get_mouse_cursor_position(void)
{
    return (qu_vec2i) { impl.input.x_mouse, impl.input.y_mouse };
}

static qu_vec2i get_mouse_cursor_delta(void)
{
    return (qu_vec2i) { impl.input.dx_mouse, impl.input.dy_mouse };
}

static qu_vec2i get_mouse_wheel_delta(void)
{
    return (qu_vec2i) { impl.input.dx_wheel, impl.input.dy_wheel };
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
    impl.callbacks.on_key_pressed = fn;
}

static void on_key_repeated(qu_key_fn fn)
{
    impl.callbacks.on_key_repeated = fn;
}

static void on_key_released(qu_key_fn fn)
{
    impl.callbacks.on_key_released = fn;
}

static void on_mouse_button_pressed(qu_mouse_button_fn fn)
{
    impl.callbacks.on_mouse_button_pressed = fn;
}

static void on_mouse_button_released(qu_mouse_button_fn fn)
{
    impl.callbacks.on_mouse_button_released = fn;
}

static void on_mouse_cursor_moved(qu_mouse_cursor_fn fn)
{
    impl.callbacks.on_mouse_cursor_moved = fn;
}

static void on_mouse_wheel_scrolled(qu_mouse_wheel_fn fn)
{
    impl.callbacks.on_mouse_wheel_scrolled = fn;
}

//------------------------------------------------------------------------------

static float get_time_mediump(void)
{
    return SDL_GetTicks() / 1000.f;
}

static double get_time_highp(void)
{
    return (double) get_time_mediump();
}

//------------------------------------------------------------------------------

void libqu_construct_emscripten_core(libqu_core *core)
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

#endif // defined(__EMSCRIPTEN__)

//------------------------------------------------------------------------------
