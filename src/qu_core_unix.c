//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#if defined(__unix__) && !defined(__EMSCRIPTEN__)

//------------------------------------------------------------------------------

#define _GNU_SOURCE

#if defined(__linux__)
#   define _POSIX_C_SOURCE 199309L
#   define _XOPEN_SOURCE 500
#endif

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>

#if defined(__linux__)
#   include <linux/joystick.h>
#   include <fcntl.h>
#   include <unistd.h>
#endif

#include "qu_core.h"
#include "qu_gateway.h"
#include "qu_halt.h"
#include "qu_log.h"

//------------------------------------------------------------------------------

#define MAX_JOYSTICKS               4
#define MAX_JOYSTICK_ID             64

#if defined(__linux__)
#   define MAX_JOYSTICK_BUTTONS    (KEY_MAX - BTN_MISC + 1)
#   define MAX_JOYSTICK_AXES       (ABS_MAX + 1)
#else
#   define MAX_JOYSTICK_BUTTONS    16
#   define MAX_JOYSTICK_AXES       16
#endif

//------------------------------------------------------------------------------

enum
{
    WM_PROTOCOLS,
    WM_DELETE_WINDOW,
    UTF8_STRING,
    NET_WM_NAME,
    TOTAL_ATOMS,
};

enum
{
    ARB_CREATE_CONTEXT = (1 << 0),
    ARB_CREATE_CONTEXT_PROFILE = (1 << 1),
    EXT_SWAP_CONTROL = (1 << 2),
    EXT_CREATE_CONTEXT_ES2_PROFILE = (1 << 3),
};

static struct
{
    bool initialized;
    bool legacy_context;

    // Clock variables

    uint64_t start_mediump;
    double start_highp;

    // Xlib variables

    Display *display;
    int screen;
    Window root;
    Colormap colormap;
    long event_mask;
    Window window;
    Atom atoms[TOTAL_ATOMS];

    // GLX-related variables

    GLXContext context;
    GLXDrawable surface;
    unsigned long extensions;

    // GLX extension function pointers

    PFNGLXSWAPINTERVALEXTPROC ext_glXSwapIntervalEXT;
    PFNGLXCREATECONTEXTATTRIBSARBPROC ext_glXCreateContextAttribsARB;

    // Window structure state

    int width;
    int height;

    // Input state

    bool keyboard_state[QU_TOTAL_KEYS];

    struct {
        uint8_t buttons;
        qu_vec2i wheel_delta;
        qu_vec2i cursor_position;
        qu_vec2i cursor_delta;
    } mouse;

    float joystick_next_poll_time;

    struct {
        char id[MAX_JOYSTICK_ID];

        int button_count;
        int axis_count;

        bool button[MAX_JOYSTICK_BUTTONS];
        float axis[MAX_JOYSTICK_AXES];

#if defined(__linux__)
        int fd;
        uint16_t button_map[MAX_JOYSTICK_BUTTONS];
        uint8_t axis_map[MAX_JOYSTICK_AXES];
#endif
    } joystick[MAX_JOYSTICKS];

    // Event handlers

    qu_key_fn on_key_pressed;
    qu_key_fn on_key_repeated;
    qu_key_fn on_key_released;
    qu_mouse_button_fn on_mouse_button_pressed;
    qu_mouse_button_fn on_mouse_button_released;
    qu_mouse_wheel_fn on_mouse_wheel_scrolled;
    qu_mouse_cursor_fn on_mouse_cursor_moved;
} impl;

//------------------------------------------------------------------------------

static qu_key key_conv(KeySym sym)
{
    switch (sym)
    {
    case XK_0:              return QU_KEY_0;
    case XK_1:              return QU_KEY_1;
    case XK_2:              return QU_KEY_2;
    case XK_3:              return QU_KEY_3;
    case XK_4:              return QU_KEY_4;
    case XK_5:              return QU_KEY_5;
    case XK_6:              return QU_KEY_6;
    case XK_7:              return QU_KEY_7;
    case XK_8:              return QU_KEY_8;
    case XK_9:              return QU_KEY_9;
    case XK_a:              return QU_KEY_A;
    case XK_b:              return QU_KEY_B;
    case XK_c:              return QU_KEY_C;
    case XK_d:              return QU_KEY_D;
    case XK_e:              return QU_KEY_E;
    case XK_f:              return QU_KEY_F;
    case XK_g:              return QU_KEY_G;
    case XK_h:              return QU_KEY_H;
    case XK_i:              return QU_KEY_I;
    case XK_j:              return QU_KEY_J;
    case XK_k:              return QU_KEY_K;
    case XK_l:              return QU_KEY_L;
    case XK_m:              return QU_KEY_M;
    case XK_n:              return QU_KEY_N;
    case XK_o:              return QU_KEY_O;
    case XK_p:              return QU_KEY_P;
    case XK_q:              return QU_KEY_Q;
    case XK_r:              return QU_KEY_R;
    case XK_s:              return QU_KEY_S;
    case XK_t:              return QU_KEY_T;
    case XK_u:              return QU_KEY_U;
    case XK_v:              return QU_KEY_V;
    case XK_w:              return QU_KEY_W;
    case XK_x:              return QU_KEY_X;
    case XK_y:              return QU_KEY_Y;
    case XK_z:              return QU_KEY_Z;
    case XK_grave:          return QU_KEY_GRAVE;
    case XK_apostrophe:     return QU_KEY_APOSTROPHE;
    case XK_minus:          return QU_KEY_MINUS;
    case XK_equal:          return QU_KEY_EQUAL;
    case XK_bracketleft:    return QU_KEY_LBRACKET;
    case XK_bracketright:   return QU_KEY_RBRACKET;
    case XK_comma:          return QU_KEY_COMMA;
    case XK_period:         return QU_KEY_PERIOD;
    case XK_semicolon:      return QU_KEY_SEMICOLON;
    case XK_slash:          return QU_KEY_SLASH;
    case XK_backslash:      return QU_KEY_BACKSLASH;
    case XK_space:          return QU_KEY_SPACE;
    case XK_Escape:         return QU_KEY_ESCAPE;
    case XK_BackSpace:      return QU_KEY_BACKSPACE;
    case XK_Tab:            return QU_KEY_TAB;
    case XK_Return:         return QU_KEY_ENTER;
    case XK_F1:             return QU_KEY_F1;
    case XK_F2:             return QU_KEY_F2;
    case XK_F3:             return QU_KEY_F3;
    case XK_F4:             return QU_KEY_F4;
    case XK_F5:             return QU_KEY_F5;
    case XK_F6:             return QU_KEY_F6;
    case XK_F7:             return QU_KEY_F7;
    case XK_F8:             return QU_KEY_F8;
    case XK_F9:             return QU_KEY_F9;
    case XK_F10:            return QU_KEY_F10;
    case XK_F11:            return QU_KEY_F11;
    case XK_F12:            return QU_KEY_F12;
    case XK_Up:             return QU_KEY_UP;
    case XK_Down:           return QU_KEY_DOWN;
    case XK_Left:           return QU_KEY_LEFT;
    case XK_Right:          return QU_KEY_RIGHT;
    case XK_Shift_L:        return QU_KEY_LSHIFT;
    case XK_Shift_R:        return QU_KEY_RSHIFT;
    case XK_Control_L:      return QU_KEY_LCTRL;
    case XK_Control_R:      return QU_KEY_RCTRL;
    case XK_Alt_L:          return QU_KEY_LALT;
    case XK_Alt_R:          return QU_KEY_RALT;
    case XK_Super_L:        return QU_KEY_LSUPER;
    case XK_Super_R:        return QU_KEY_RSUPER;
    case XK_Menu:           return QU_KEY_MENU;
    case XK_Page_Up:        return QU_KEY_PGUP;
    case XK_Page_Down:      return QU_KEY_PGDN;
    case XK_Home:           return QU_KEY_HOME;
    case XK_End:            return QU_KEY_END;
    case XK_Insert:         return QU_KEY_INSERT;
    case XK_Delete:         return QU_KEY_DELETE;
    case XK_Print:          return QU_KEY_PRINTSCREEN;
    case XK_Pause:          return QU_KEY_PAUSE;
    case XK_Caps_Lock:      return QU_KEY_CAPSLOCK;
    case XK_Scroll_Lock:    return QU_KEY_SCROLLLOCK;
    case XK_Num_Lock:       return QU_KEY_NUMLOCK;
    case XK_KP_0:           return QU_KEY_KP_0;
    case XK_KP_1:           return QU_KEY_KP_1;
    case XK_KP_2:           return QU_KEY_KP_2;
    case XK_KP_3:           return QU_KEY_KP_3;
    case XK_KP_4:           return QU_KEY_KP_4;
    case XK_KP_5:           return QU_KEY_KP_5;
    case XK_KP_6:           return QU_KEY_KP_6;
    case XK_KP_7:           return QU_KEY_KP_7;
    case XK_KP_8:           return QU_KEY_KP_8;
    case XK_KP_9:           return QU_KEY_KP_9;
    case XK_KP_Multiply:
        return QU_KEY_KP_MUL;
    case XK_KP_Add:
        return QU_KEY_KP_ADD;
    case XK_KP_Subtract:
        return QU_KEY_KP_SUB;
    case XK_KP_Decimal:
        return QU_KEY_KP_POINT;
    case XK_KP_Divide:
        return QU_KEY_KP_DIV;
    case XK_KP_Enter:
        return QU_KEY_KP_ENTER;
    default:
        return QU_KEY_INVALID;
    }
}

static qu_mouse_button mouse_button_conv(unsigned int button)
{
    switch (button) {
    case Button1:           return QU_MOUSE_BUTTON_LEFT;
    case Button2:           return QU_MOUSE_BUTTON_MIDDLE;
    case Button3:           return QU_MOUSE_BUTTON_RIGHT;
    default:                return QU_MOUSE_BUTTON_INVALID;
    }
}

static void handle_key_press(XKeyEvent *xkey)
{
    qu_key key = key_conv(XLookupKeysym(xkey, ShiftMapIndex));

    if (key == QU_KEY_INVALID) {
        return;
    }

    if (impl.keyboard_state[key]) {
        if (impl.on_key_repeated) {
            impl.on_key_repeated(key);
        }
    } else {
        impl.keyboard_state[key] = true;

        if (impl.on_key_pressed) {
            impl.on_key_pressed(key);
        }
    }
}

static void handle_key_release(XKeyEvent *xkey)
{
    qu_key key = key_conv(XLookupKeysym(xkey, ShiftMapIndex));

    if (key == QU_KEY_INVALID) {
        return;
    }

    impl.keyboard_state[key] = false;

    if (impl.on_key_released) {
        impl.on_key_released(key);
    }
}

static void handle_mouse_button_press(XButtonEvent *xbutton)
{
    qu_mouse_button button = mouse_button_conv(xbutton->button);

    if (button == QU_MOUSE_BUTTON_INVALID) {
        return;
    }

    impl.mouse.buttons |= (1 << button);

    if (impl.on_mouse_button_pressed) {
        impl.on_mouse_button_pressed(button);
    }
}

static void handle_mouse_button_release(XButtonEvent *xbutton)
{
    qu_mouse_button button = mouse_button_conv(xbutton->button);

    if (button == QU_MOUSE_BUTTON_INVALID) {
        return;
    }

    impl.mouse.buttons &= ~(1 << button);

    if (impl.on_mouse_button_released) {
        impl.on_mouse_button_released(button);
    }
}

#if defined(__linux__)

static void handle_linux_joystick_event(int joystick, struct js_event *event)
{
    // Event type can be JS_EVENT_BUTTON or JS_EVENT_AXIS.
    // Either of these events can be ORd together with JS_EVENT_INIT
    // if event is sent for the first time in order to give you
    // initial values for buttons and axes.

    if (event->type & JS_EVENT_BUTTON) {
        impl.joystick[joystick].button[event->number] = event->value;

        if (!(event->type & JS_EVENT_INIT)) {
            // TODO: ???
        }
    } else if (event->type & JS_EVENT_AXIS) {
        impl.joystick[joystick].axis[event->number] = event->value / 32767.f;
    }
}

#endif

//------------------------------------------------------------------------------

static void set_title(char const *title)
{
    XStoreName(impl.display, impl.window, title);
    XChangeProperty(impl.display, impl.window,
        impl.atoms[NET_WM_NAME],
        impl.atoms[UTF8_STRING],
        8, PropModeReplace,
        (unsigned char const *) title, strlen(title));
}

static void set_display_size(int width, int height)
{
    int x = (DisplayWidth(impl.display, impl.screen) / 2) - (width / 2);
    int y = (DisplayHeight(impl.display, impl.screen) / 2) - (height / 2);

    XMoveResizeWindow(impl.display, impl.window, x, y, width, height);

#if 0
    XSizeHints hints = {
        .flags = PMinSize | PMaxSize,
        .min_width = params->display_width,
        .max_width = params->display_width,
        .min_height = params->display_height,
        .max_height = params->display_height,
    };

    XMoveWindow(impl.display, impl.window, x, y);
    XSetWMNormalHints(impl.display, impl.window, &hints);
#endif
}

static void initialize(qu_params const *params)
{
    memset(&impl, 0, sizeof(impl));

    // (?) Initialize clock

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    impl.start_mediump = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    impl.start_highp = (double) ts.tv_sec + (ts.tv_nsec / 1.0e9);

    // (0) Open display

    impl.display = XOpenDisplay(NULL);

    if (!impl.display) {
        libqu_error("Failed to open X11 display.");
        return;
    }

    impl.screen = DefaultScreen(impl.display);
    impl.root = DefaultRootWindow(impl.display);

    // (1) Choose GLX framebuffer configuration

    int fbc_attribs[] = {
        GLX_RED_SIZE,       (8),
        GLX_GREEN_SIZE,     (8),
        GLX_BLUE_SIZE,      (8),
        GLX_ALPHA_SIZE,     (8),
        GLX_RENDER_TYPE,    (GLX_RGBA_BIT),
        GLX_DRAWABLE_TYPE,  (GLX_WINDOW_BIT),
        GLX_X_RENDERABLE,   (True),
        GLX_DOUBLEBUFFER,   (True),
        None,
    };

    int fbc_total = 0;
    GLXFBConfig *fbc_list = glXChooseFBConfig(impl.display, impl.screen,
                                              fbc_attribs, &fbc_total);

    if (!fbc_list || !fbc_total) {
        libqu_error("No suitable framebuffer configuration found.");
        return;
    }

    int best_fbc = -1;
    int best_sample_count = 0;

    for (int i = 0; i < fbc_total; i++) {
        int has_sample_buffers = 0;
        glXGetFBConfigAttrib(impl.display, fbc_list[i], GLX_SAMPLE_BUFFERS, &has_sample_buffers);

        if (!has_sample_buffers) {
            continue;
        }

        int total_samples = 0;
        glXGetFBConfigAttrib(impl.display, fbc_list[i], GLX_SAMPLES, &total_samples);

        if (total_samples > best_sample_count) {
            best_fbc = i;
            best_sample_count = total_samples;
        }
    }

    if (best_fbc >= 0) {
        libqu_info("Selected FBConfig with %d samples.\n", best_sample_count);
    }

    GLXFBConfig fbconfig = fbc_list[(best_fbc < 0) ? 0 : best_fbc];
    XFree(fbc_list);

    // (2) Create window

    XVisualInfo *vi = glXGetVisualFromFBConfig(impl.display, fbconfig);

    impl.colormap = XCreateColormap(impl.display, impl.root, vi->visual, AllocNone);
    impl.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
        StructureNotifyMask;

    impl.window = XCreateWindow(
        impl.display,   // display
        impl.root,      // parent
        0, 0,           // position
        params->display_width,
        params->display_height,
        0,              // border width
        CopyFromParent, // depth
        InputOutput,    // class
        vi->visual,     // visual
        CWColormap | CWEventMask,
        &(XSetWindowAttributes) {
            .colormap = impl.colormap,
            .event_mask = impl.event_mask,
        }
    );

    if (!impl.window) {
        libqu_error("Failed to create X11 window.");
        return;
    }

    // (3) Store X11 atoms and set protocols

    char *atom_names[TOTAL_ATOMS] = {
        [WM_PROTOCOLS] = "WM_PROTOCOLS",
        [WM_DELETE_WINDOW] = "WM_DELETE_WINDOW",
        [UTF8_STRING] = "UTF8_STRING",
        [NET_WM_NAME] = "_NET_WM_NAME",
    };

    XInternAtoms(impl.display, atom_names, TOTAL_ATOMS, False, impl.atoms);

    Atom protocols[] = { impl.atoms[WM_DELETE_WINDOW] };
    XSetWMProtocols(impl.display, impl.window, protocols, 1);

    // (4) WM_CLASS

    XClassHint *class_hint = XAllocClassHint();
    class_hint->res_class = "libquack application";
    XSetClassHint(impl.display, impl.window, class_hint);
    XFree(class_hint);

    // (5) Parse GLX extension list

    char *glx_extension_list = strdup(glXQueryExtensionsString(impl.display, impl.screen));
    char *glx_extension = strtok(glx_extension_list, " ");

    while (glx_extension) {
        if (!strcmp(glx_extension, "GLX_ARB_create_context")) {
            impl.extensions |= ARB_CREATE_CONTEXT;
            impl.ext_glXCreateContextAttribsARB =
                (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB(
                    (GLubyte const *)"glXCreateContextAttribsARB");

            libqu_info("GLX_ARB_create_context is supported.\n");
        } else if (!strcmp(glx_extension, "GLX_ARB_create_context_profile")) {
            impl.extensions |= ARB_CREATE_CONTEXT_PROFILE;

            libqu_info("GLX_ARB_create_context_profile is supported.\n");
        } else if (!strcmp(glx_extension, "GLX_EXT_swap_control")) {
            impl.extensions |= EXT_SWAP_CONTROL;
            impl.ext_glXSwapIntervalEXT =
                (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB(
                    (GLubyte const *)"glXSwapIntervalEXT");

            libqu_info("GLX_EXT_swap_control is supported.\n");
        } else if (!strcmp(glx_extension, "GLX_EXT_create_context_es2_profile")) {
            impl.extensions |= EXT_CREATE_CONTEXT_ES2_PROFILE;
            libqu_info("GLX_EXT_create_context_es2_profile is supported.\n");
        }

        glx_extension = strtok(NULL, " ");
    }

    free(glx_extension_list);

    // (6) Create GLX context and surface

    char *env = getenv("LIBQU_FORCE_LEGACY_GL");
    bool use_es2 = true;

    if (env) {
        if (!strcmp(env, "1") || !strcasecmp(env, "YES")) {
            use_es2 = false;
        }
    }

    if (use_es2 && (impl.extensions & EXT_CREATE_CONTEXT_ES2_PROFILE)) {
        libqu_info("Creating OpenGL ES 2.0 context...\n");

        int ctx_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB,  2,
            GLX_CONTEXT_MINOR_VERSION_ARB,  0,
            GLX_CONTEXT_PROFILE_MASK_ARB,   GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
            None,
        };

        impl.context = impl.ext_glXCreateContextAttribsARB(
            impl.display, fbconfig, NULL, True, ctx_attribs
        );
    }

    if (!impl.context) {
        libqu_info("Creating legacy OpenGL context...\n");
        impl.context = glXCreateContext(impl.display, vi, NULL, True);
        impl.legacy_context = true;
    }

    XFree(vi);

    if (!impl.context) {
        libqu_error("Failed to create OpenGL context.");
        return;
    }

    impl.surface = glXCreateWindow(impl.display, fbconfig, impl.window, NULL);
    glXMakeContextCurrent(impl.display, impl.surface, impl.surface, impl.context);

    // (7) Set detectable key auto-repeat

    XkbSetDetectableAutoRepeat(impl.display, True, NULL);

    // (8) Set title and size of the window

    XMapWindow(impl.display, impl.window);

    set_title(params->title);
    set_display_size(params->display_width, params->display_height);

    XSync(impl.display, False);

    // (8.1) ...

    XWindowAttributes xwa;
    XGetWindowAttributes(impl.display, impl.window, &xwa);

    impl.width = xwa.width;
    impl.height = xwa.height;

    // (8.2) Joystick

#if defined(__linux__)

    for (int i = 0; i < MAX_JOYSTICKS; i++) {
        impl.joystick[i].fd = -1;
    }

#endif

    // (9) Done.

    libqu_info("Xlib-based core module initialized.\n");
    impl.initialized = true;
}

static void terminate(void)
{
    if (impl.display) {
        glXDestroyWindow(impl.display, impl.surface);
        glXDestroyContext(impl.display, impl.context);

        XDestroyWindow(impl.display, impl.window);
        XFreeColormap(impl.display, impl.colormap);
        XCloseDisplay(impl.display);
    }

    if (impl.initialized) {
        libqu_info("Xlib-based core module terminated.\n");
        impl.initialized = false;
    }
}

static bool is_initialized(void)
{
    return impl.initialized;
}

static bool process(void)
{
    XEvent event;

    while (XCheckTypedWindowEvent(impl.display, impl.window, ClientMessage, &event)) {
        if (event.xclient.data.l[0] == (long) impl.atoms[WM_DELETE_WINDOW]) {
            return false;
        }
    }

    impl.mouse.cursor_delta.x = 0;
    impl.mouse.cursor_delta.y = 0;

    impl.mouse.wheel_delta.x = 0;
    impl.mouse.wheel_delta.y = 0;

    while (XCheckWindowEvent(impl.display, impl.window, impl.event_mask, &event)) {
        switch (event.type) {
        case Expose:
            break;
        case KeyPress:
            handle_key_press(&event.xkey);
            break;
        case KeyRelease:
            handle_key_release(&event.xkey);
            break;
        case MotionNotify:
            impl.mouse.cursor_delta.x = event.xmotion.x - impl.mouse.cursor_position.x;
            impl.mouse.cursor_delta.y = event.xmotion.y - impl.mouse.cursor_position.y;
            impl.mouse.cursor_position.x = event.xmotion.x;
            impl.mouse.cursor_position.y = event.xmotion.y;
            break;
        case ButtonPress:
            if (event.xbutton.button == Button4) {
                impl.mouse.wheel_delta.y++;
            } else if (event.xbutton.button == Button5) {
                impl.mouse.wheel_delta.y--;
            } else if (event.xbutton.button == 6) {
                impl.mouse.wheel_delta.x--;
            } else if (event.xbutton.button == 7) {
                impl.mouse.wheel_delta.x++;
            } else {
                handle_mouse_button_press(&event.xbutton);
            }
            break;
        case ButtonRelease:
            handle_mouse_button_release(&event.xbutton);
            break;
        case ConfigureNotify:
            if (event.xconfigure.width != impl.width || event.xconfigure.height != impl.height) {
                impl.width = event.xconfigure.width;
                impl.height = event.xconfigure.height;
                libqu_notify_display_resize(impl.width, impl.height);
            }

            break;
        default:
            libqu_debug("Unhandled event: 0x%04x\n", event.type);
            break;
        }
    }

    if (impl.mouse.wheel_delta.x || impl.mouse.wheel_delta.y) {
        if (impl.on_mouse_wheel_scrolled) {
            impl.on_mouse_wheel_scrolled(
                impl.mouse.wheel_delta.x, impl.mouse.wheel_delta.y
            );
        }
    }

    if (impl.mouse.cursor_delta.x || impl.mouse.cursor_delta.y) {
        if (impl.on_mouse_cursor_moved) {
            impl.on_mouse_cursor_moved(
                impl.mouse.cursor_delta.x, impl.mouse.cursor_delta.y
            );
        }
    }

#if defined(__linux__)

    // Read joystick events.
    for (int i = 0; i < MAX_JOYSTICKS; i++) {
        if (impl.joystick[i].fd == -1) {
            continue;
        }

        struct js_event event;

        // -1 is returned if event queue is empty.
        while (read(impl.joystick[i].fd, &event, sizeof(event)) != -1) {
            handle_linux_joystick_event(i, &event);
        }

        // errno is set to EAGAIN if joystick is still connected.
        // If it's not, then joystick has been disconnected.
        if (errno != EAGAIN) {
            close(impl.joystick[i].fd);

            libqu_info("Joystick '%s' disconnected.\n", impl.joystick[i].id);

            memset(&impl.joystick[i], 0, sizeof(impl.joystick[i]));

            impl.joystick[i].fd = -1;
            impl.joystick[i].button_count = 0;
            impl.joystick[i].axis_count = 0;
        }
    }

#endif // defined(__linux__)

    return true;
}

static void present(void)
{
    glXSwapBuffers(impl.display, impl.surface);
}

static libqu_gc get_gc(void)
{
    return impl.legacy_context ? LIBQU_GC_GL : LIBQU_GC_GLES;
}

static bool gl_check_extension(char const *name)
{
    char *list = strdup(glXQueryExtensionsString(impl.display, impl.screen));
    char *token = strtok(list, " ");

    while (token) {
        if (strcmp(token, name) == 0) {
            free(list);
            return true;
        }

        token = strtok(NULL, " ");
    }

    free(list);
    return false;
}

static void *gl_proc_address(char const *name)
{
    return glXGetProcAddress((GLubyte const *) name);
}

//------------------------------------------------------------------------------

static bool const *get_keyboard_state(void)
{
    return impl.keyboard_state;
}

static bool is_key_pressed(qu_key key)
{
    return impl.keyboard_state[key];
}

//------------------------------------------------------------------------------

static uint8_t get_mouse_button_state(void)
{
    return impl.mouse.buttons;
}

static bool is_mouse_button_pressed(qu_mouse_button button)
{
    return impl.mouse.buttons & (1 << button);
}

static qu_vec2i get_mouse_cursor_position(void)
{
    return impl.mouse.cursor_position;
}

static qu_vec2i get_mouse_cursor_delta(void)
{
    return impl.mouse.cursor_delta;
}

static qu_vec2i get_mouse_wheel_delta(void)
{
    return impl.mouse.wheel_delta;
}

//------------------------------------------------------------------------------

#if defined(__linux__)

static bool is_joystick_connected(int joystick)
{
    if (joystick < 0 || joystick >= MAX_JOYSTICKS) {
        return false;
    }

    if (impl.joystick[joystick].fd != -1) {
        return true;
    }

    float current_time = qu_get_time_mediump();

    if (impl.joystick_next_poll_time > current_time) {
        return false;
    }

    impl.joystick_next_poll_time = current_time + 1.f;

    char path[64];
    snprintf(path, sizeof(path) - 1, "/dev/input/js%d", joystick);

    int fd = open(path, O_RDONLY | O_NONBLOCK);

    if (fd == -1) {
        return false;
    }

    ioctl(fd, JSIOCGNAME(MAX_JOYSTICK_ID), impl.joystick[joystick].id);
    ioctl(fd, JSIOCGBUTTONS, &impl.joystick[joystick].button_count);
    ioctl(fd, JSIOCGAXES, &impl.joystick[joystick].axis_count);
    ioctl(fd, JSIOCGBTNMAP, impl.joystick[joystick].button_map);
    ioctl(fd, JSIOCGAXMAP, impl.joystick[joystick].axis_map);

    impl.joystick[joystick].fd = fd;

    libqu_info("Joystick '%s' connected.\n", impl.joystick[joystick].id);
    libqu_info("# of buttons: %d.\n", impl.joystick[joystick].button_count);
    libqu_info("# of axes: %d.\n", impl.joystick[joystick].axis_count);

    return true;
}

static char const *get_joystick_button_id(int joystick, int button)
{
    if (joystick < 0 || joystick >= MAX_JOYSTICKS) {
        return NULL;
    }

    if (button < 0 || button >= MAX_JOYSTICK_BUTTONS) {
        return NULL;
    }

    uint16_t index = impl.joystick[joystick].button_map[button];

    switch (index) {
    case BTN_TRIGGER:       return "TRIGGER";
    case BTN_THUMB:         return "THUMB";
    case BTN_THUMB2:        return "THUMB2";
    case BTN_TOP:           return "TOP";
    case BTN_TOP2:          return "TOP2";
    case BTN_PINKIE:        return "PINKIE";
    case BTN_BASE:          return "BASE";
    case BTN_BASE2:         return "BASE2";
    case BTN_BASE3:         return "BASE3";
    case BTN_BASE4:         return "BASE4";
    case BTN_BASE5:         return "BASE5";
    case BTN_BASE6:         return "BASE6";
    case BTN_DEAD:          return "DEAD";
    case BTN_A:             return "A"; // also SOUTH
    case BTN_B:             return "B"; // also EAST
    case BTN_C:             return "C";
    case BTN_X:             return "X"; // also NORTH
    case BTN_Y:             return "Y"; // also WEST
    case BTN_Z:             return "Z";
    case BTN_TL:            return "TL";
    case BTN_TR:            return "TR";
    case BTN_TL2:           return "TL2";
    case BTN_TR2:           return "TR2";
    case BTN_SELECT:        return "SELECT";
    case BTN_START:         return "START";
    case BTN_MODE:          return "MODE";
    case BTN_THUMBL:        return "THUMBL";
    case BTN_THUMBR:        return "THUMBR";
    default:
        break;
    }

    return NULL;
}

static char const *get_joystick_axis_id(int joystick, int axis)
{
    if (joystick < 0 || joystick >= MAX_JOYSTICKS) {
        return NULL;
    }

    if (axis < 0 || axis >= MAX_JOYSTICK_AXES) {
        return NULL;
    }

    uint16_t index = impl.joystick[joystick].axis_map[axis];

    switch (index) {
    case ABS_X:             return "X";
    case ABS_Y:             return "Y";
    case ABS_Z:             return "Z";
    case ABS_RX:            return "RX";
    case ABS_RY:            return "RY";
    case ABS_RZ:            return "RZ";
    case ABS_THROTTLE:      return "THROTTLE";
    case ABS_RUDDER:        return "RUDDER";
    case ABS_WHEEL:         return "WHEEL";
    case ABS_GAS:           return "GAS";
    case ABS_BRAKE:         return "BRAKE";
    case ABS_HAT0X:         return "HAT0X";
    case ABS_HAT0Y:         return "HAT0Y";
    case ABS_HAT1X:         return "HAT1X";
    case ABS_HAT1Y:         return "HAT1Y";
    case ABS_HAT2X:         return "HAT2X";
    case ABS_HAT2Y:         return "HAT2Y";
    case ABS_HAT3X:         return "HAT3X";
    case ABS_HAT3Y:         return "HAT3Y";
    case ABS_PRESSURE:      return "PRESSURE";
    case ABS_DISTANCE:      return "DISTANCE";
    case ABS_TILT_X:        return "TILT_X";
    case ABS_TILT_Y:        return "TILT_Y";
    case ABS_TOOL_WIDTH:    return "TOOL_WIDTH";
    case ABS_VOLUME:        return "VOLUME";
    case ABS_MISC:          return "MISC";
    default:
        break;
    }

    return NULL;
}

#else

static bool is_joystick_connected(int joystick)
{
    return false;
}

static char const *get_joystick_button_id(int joystick, int button)
{
    return NULL;
}

static char const *get_joystick_axis_id(int joystick, int axis)
{
    return NULL;
}

#endif

static char const *get_joystick_id(int joystick)
{
    if (joystick < 0 || joystick >= MAX_JOYSTICKS) {
        return NULL;
    }

    return impl.joystick[joystick].id;
}

static int get_joystick_button_count(int joystick)
{
    if (joystick < 0 || joystick >= MAX_JOYSTICKS) {
        return 0;
    }

    return impl.joystick[joystick].button_count;
}

static int get_joystick_axis_count(int joystick)
{
    if (joystick < 0 || joystick >= MAX_JOYSTICKS) {
        return 0;
    }

    return impl.joystick[joystick].axis_count;
}

static bool is_joystick_button_pressed(int joystick, int button)
{
    if (joystick < 0 || joystick >= MAX_JOYSTICKS) {
        return false;
    }

    if (button < 0 || button >= MAX_JOYSTICK_BUTTONS) {
        return false;
    }

    return impl.joystick[joystick].button[button];
}

static float get_joystick_axis_value(int joystick, int axis)
{
    if (joystick < 0 || joystick >= MAX_JOYSTICKS) {
        return 0.f;
    }

    if (axis < 0 || axis >= MAX_JOYSTICK_AXES) {
        return 0.f;
    }

    return impl.joystick[joystick].axis[axis];
}

//------------------------------------------------------------------------------

static void on_key_pressed(qu_key_fn fn)
{
    impl.on_key_pressed = fn;
}

static void on_key_repeated(qu_key_fn fn)
{
    impl.on_key_repeated = fn;
}

static void on_key_released(qu_key_fn fn)
{
    impl.on_key_released = fn;
}

static void on_mouse_button_pressed(qu_mouse_button_fn fn)
{
    impl.on_mouse_button_pressed = fn;
}

static void on_mouse_button_released(qu_mouse_button_fn fn)
{
    impl.on_mouse_button_released = fn;
}

static void on_mouse_cursor_moved(qu_mouse_cursor_fn fn)
{
    impl.on_mouse_cursor_moved = fn;
}

static void on_mouse_wheel_scrolled(qu_mouse_wheel_fn fn)
{
    impl.on_mouse_wheel_scrolled = fn;
}

//------------------------------------------------------------------------------

static float get_time_mediump(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    unsigned long long msec = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    return (msec - impl.start_mediump) / 1000.0f;
}

static double get_time_highp(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (double) ts.tv_sec + (ts.tv_nsec / 1.0e9) - impl.start_highp;
}

//------------------------------------------------------------------------------

void libqu_construct_unix_core(libqu_core *core)
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
        .get_joystick_button_id = get_joystick_button_id,
        .get_joystick_axis_id = get_joystick_axis_id,
        .is_joystick_button_pressed = is_joystick_button_pressed,
        .get_joystick_axis_value = get_joystick_axis_value,
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

#endif // defined(__unix__) && !defined(__EMSCRIPTEN__)

//------------------------------------------------------------------------------
