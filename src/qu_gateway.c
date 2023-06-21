//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <stdarg.h>
#include <string.h>

#if defined(__EMSCRIPTEN__)
#   include <emscripten.h>
#endif

#include "qu_audio.h"
#include "qu_core.h"
#include "qu_graphics.h"
#include "qu_gateway.h"
#include "qu_halt.h"
#include "qu_log.h"
#include "qu_platform.h"
#include "qu_text.h"

//------------------------------------------------------------------------------

static struct
{
    qu_params params;
    bool initialized;
    libqu_core core;
    libqu_graphics graphics;
    libqu_audio audio;
    libqu_gc gc;
} qu;

//------------------------------------------------------------------------------

static void initialize_graphics(libqu_gc gc)
{
    if (gc == qu.gc) {
        return;
    }

    switch (gc) {
    default:
        libqu_construct_null_graphics(&qu.graphics);
        break;

#if !defined(QU_DISABLE_GL)
    case LIBQU_GC_GL:
        libqu_construct_gl2_graphics(&qu.graphics);
        break;
#endif

#if !defined(QU_DISABLE_GLES2)
    case LIBQU_GC_GLES:
        libqu_construct_gles2_graphics(&qu.graphics);
        break;
#endif
    }

    qu.gc = gc;
    qu.graphics.initialize(&qu.params);

    if (!qu.graphics.is_initialized()) {
        qu.graphics.terminate();
        qu.gc = LIBQU_GC_NONE;

        libqu_construct_null_graphics(&qu.graphics);
        libqu_error("Failed to initialize graphics module, falling back to dummy.\n");
    }
}

//------------------------------------------------------------------------------

void qu_initialize(qu_params const *user_params)
{
    if (qu.initialized) {
        libqu_warning("Attempt to initialize multiple times.\n");
        return;
    }

    libqu_platform_initialize();

#if defined(_WIN32)
    libqu_construct_win32_core(&qu.core);
#elif defined(__EMSCRIPTEN__)
    libqu_construct_emscripten_core(&qu.core);
#elif defined(__ANDROID__)
    libqu_construct_android_core(&qu.core);
#elif defined(__unix__)
    libqu_construct_unix_core(&qu.core);
#else
    libqu_construct_null_core(&qu.core);
#endif

    memset(&qu.params, 0, sizeof(qu.params));

    if (user_params) {
        memcpy(&qu.params, user_params, sizeof(qu_params));
    } else {
        memset(&qu.params, 0, sizeof(qu_params));
    }

    qu.gc = -1;

    if (!qu.params.title) {
        qu.params.title = "libquack";
    }

    if (!qu.params.display_width || !qu.params.display_height) {
        qu.params.display_width = 720;
        qu.params.display_height = 480;
    }

    if (!qu.params.canvas_width || !qu.params.canvas_height) {
        qu.params.canvas_width = qu.params.display_width;
        qu.params.canvas_height = qu.params.display_height;
    }

    qu.core.initialize(&qu.params);

    if (!qu.core.is_initialized()) {
        libqu_halt("Failed to initialize core module.\n");
    }

    initialize_graphics(qu.core.get_gc());
    libqu_initialize_text(&qu.graphics);

    libqu_construct_openal_audio(&qu.audio);
    qu.audio.initialize(&qu.params);

    if (!qu.audio.is_initialized()) {
        qu.audio.terminate();

        libqu_error("Failed to initialize audio module, falling back to dummy.\n");
        libqu_construct_null_audio(&qu.audio);
    }

    qu.initialized = true;
}

void qu_terminate(void)
{
    if (!qu.initialized) {
        libqu_warning("Can't terminate; not initialized.\n");
        return;
    }

    libqu_terminate_text();

    qu.audio.terminate();
    qu.graphics.terminate();
    qu.core.terminate();

    libqu_platform_terminate();

    memset(&qu, 0, sizeof(qu));
}

bool qu_process(void)
{
    return qu.core.process();
}

#if defined(__EMSCRIPTEN__)

static void main_loop(void *callback_pointer)
{
    if (qu_process()) {
        qu_loop_fn loop_fn = callback_pointer;
        loop_fn();
    } else {
        qu_terminate();
        emscripten_cancel_main_loop();
    }
}

void qu_execute(qu_loop_fn loop_fn)
{
    emscripten_set_main_loop_arg(main_loop, loop_fn, 0, 1);
    exit(EXIT_SUCCESS);
}

#else

void qu_execute(qu_loop_fn loop_fn)
{
    while (qu_process() && loop_fn()) {
        // Intentionally left blank
    }

    qu_terminate();
    exit(EXIT_SUCCESS);
}

#endif

void qu_present(void)
{
    qu.graphics.swap();
    qu.core.present();
}

//------------------------------------------------------------------------------

bool libqu_gl_check_extension(char const *name)
{
    return qu.core.gl_check_extension(name);
}

void *libqu_gl_proc_address(char const *name)
{
    return qu.core.gl_proc_address(name);
}

void libqu_notify_gc_created(libqu_gc gc)
{
    libqu_terminate_text();
    initialize_graphics(gc);
    libqu_initialize_text(&qu.graphics);
}

void libqu_notify_gc_destroyed(void)
{
    libqu_construct_null_graphics(&qu.graphics);
}

//------------------------------------------------------------------------------

bool const *qu_get_keyboard_state(void)
{
    return qu.core.get_keyboard_state();
}

bool qu_is_key_pressed(qu_key key)
{
    return qu.core.is_key_pressed(key);
}

uint8_t qu_get_mouse_button_state(void)
{
    return qu.core.get_mouse_button_state();
}

bool qu_is_mouse_button_pressed(qu_mouse_button button)
{
    return qu.core.is_mouse_button_pressed(button);
}

qu_vec2i qu_get_mouse_cursor_position(void)
{
    return qu.graphics.conv_cursor(qu.core.get_mouse_cursor_position());
}

qu_vec2i qu_get_mouse_cursor_delta(void)
{
    return qu.graphics.conv_cursor_delta(qu.core.get_mouse_cursor_delta());
}

qu_vec2i qu_get_mouse_wheel_delta(void)
{
    return qu.core.get_mouse_wheel_delta();
}

bool qu_is_joystick_connected(int joystick)
{
    return qu.core.is_joystick_connected(joystick);
}

char const *qu_get_joystick_id(int joystick)
{
    return qu.core.get_joystick_id(joystick);
}

int qu_get_joystick_button_count(int joystick)
{
    return qu.core.get_joystick_button_count(joystick);
}

int qu_get_joystick_axis_count(int joystick)
{
    return qu.core.get_joystick_axis_count(joystick);
}

char const *qu_get_joystick_button_id(int joystick, int button)
{
    return qu.core.get_joystick_button_id(joystick, button);
}

char const *qu_get_joystick_axis_id(int joystick, int axis)
{
    return qu.core.get_joystick_axis_id(joystick, axis);
}

bool qu_is_joystick_button_pressed(int joystick, int button)
{
    return qu.core.is_joystick_button_pressed(joystick, button);
}

float qu_get_joystick_axis_value(int joystick, int axis)
{
    return qu.core.get_joystick_axis_value(joystick, axis);
}

void qu_on_key_pressed(qu_key_fn fn)
{
    qu.core.on_key_pressed(fn);
}

void qu_on_key_repeated(qu_key_fn fn)
{
    qu.core.on_key_repeated(fn);
}

void qu_on_key_released(qu_key_fn fn)
{
    qu.core.on_key_released(fn);
}

void qu_on_mouse_button_pressed(qu_mouse_button_fn fn)
{
    qu.core.on_mouse_button_pressed(fn);
}

void qu_on_mouse_button_released(qu_mouse_button_fn fn)
{
    qu.core.on_mouse_button_released(fn);
}

void qu_on_mouse_cursor_moved(qu_mouse_cursor_fn fn)
{
    qu.core.on_mouse_cursor_moved(fn);
}

void qu_on_mouse_wheel_scrolled(qu_mouse_wheel_fn fn)
{
    qu.core.on_mouse_wheel_scrolled(fn);
}

float qu_get_time_mediump(void)
{
    return libqu_get_time_mediump();
}

double qu_get_time_highp(void)
{
    return libqu_get_time_highp();
}

//------------------------------------------------------------------------------

void libqu_notify_display_resize(int width, int height)
{
    qu.graphics.notify_display_resize(width, height);
}

void qu_set_view(float x, float y, float w, float h, float rotation)
{
    qu.graphics.set_view(x, y, w, h, rotation);
}

void qu_reset_view(void)
{
    qu.graphics.reset_view();
}

void qu_push_matrix(void)
{
    qu.graphics.push_matrix();
}

void qu_pop_matrix(void)
{
    qu.graphics.pop_matrix();
}

void qu_translate(float x, float y)
{
    qu.graphics.translate(x, y);
}

void qu_scale(float x, float y)
{
    qu.graphics.scale(x, y);
}

void qu_rotate(float degrees)
{
    qu.graphics.rotate(degrees);
}

void qu_clear(qu_color color)
{
    qu.graphics.clear(color);
}

void qu_draw_point(float x, float y, qu_color color)
{
    qu.graphics.draw_point(x, y, color);
}

void qu_draw_line(float ax, float ay, float bx, float by, qu_color color)
{
    qu.graphics.draw_line(ax, ay, bx, by, color);
}

void qu_draw_triangle(float ax, float ay, float bx, float by,
                      float cx, float cy, qu_color outline, qu_color fill)
{
    qu.graphics.draw_triangle(ax, ay, bx, by, cx, cy, outline, fill);
}

void qu_draw_rectangle(float x, float y, float w, float h, qu_color outline,
                       qu_color fill)
{
    qu.graphics.draw_rectangle(x, y, w, h, outline, fill);
}

void qu_draw_circle(float x, float y, float radius,
                    qu_color outline, qu_color fill)
{
    qu.graphics.draw_circle(x, y, radius, outline, fill);
}

qu_texture qu_load_texture(char const *path)
{
    libqu_file *file = libqu_fopen(path);

    if (!file) {
        return (qu_texture) { 0 };
    }

    return (qu_texture) { qu.graphics.load_texture(file) };
}

void qu_delete_texture(qu_texture texture)
{
    qu.graphics.delete_texture(texture.id);
}

void qu_set_texture_smooth(qu_texture texture, bool smooth)
{
    qu.graphics.set_texture_smooth(texture.id, smooth);
}

void qu_draw_texture(qu_texture texture, float x, float y, float w, float h)
{
    qu.graphics.draw_texture(texture.id, x, y, w, h);
}

void qu_draw_subtexture(qu_texture texture, float x, float y, float w, float h, float rx, float ry, float rw, float rh)
{
    qu.graphics.draw_subtexture(texture.id, x, y, w, h, rx, ry, rw, rh);
}

qu_font qu_load_font(char const *path, float pt)
{
    libqu_file *file = libqu_fopen(path);

    if (file) {
        int32_t id = libqu_load_font(file, pt);

        if (id == 0) {
            libqu_fclose(file);
        }

        return (qu_font) { id };
    }

    return (qu_font) { 0 };
}

void qu_delete_font(qu_font font)
{
    libqu_delete_font(font.id);
}

void qu_draw_text(qu_font font, float x, float y, qu_color color, char const *str)
{
    libqu_draw_text(font.id, x, y, color, str);
}

void qu_draw_text_fmt(qu_font font, float x, float y, qu_color color, char const *fmt, ...)
{
    va_list ap;
    char buffer[256];
    char *heap = NULL;

    va_start(ap, fmt);
    int required = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    if ((size_t) required >= sizeof(buffer)) {
        heap = malloc(required + 1);

        if (heap) {
            va_start(ap, fmt);
            vsnprintf(heap, required + 1, fmt, ap);
            va_end(ap);
        }
    }

    libqu_draw_text(font.id, x, y, color, heap ? heap : buffer);
    free(heap);
}

qu_surface qu_create_surface(int width, int height)
{
    return (qu_surface) { qu.graphics.create_surface(width, height) };
}

void qu_delete_surface(qu_surface surface)
{
    qu.graphics.delete_surface(surface.id);
}

void qu_set_surface(qu_surface surface)
{
    qu.graphics.set_surface(surface.id);
}

void qu_reset_surface(void)
{
    qu.graphics.reset_surface();
}

void qu_draw_surface(qu_surface surface, float x, float y, float w, float h)
{
    qu.graphics.draw_surface(surface.id, x, y, w, h);
}

//------------------------------------------------------------------------------

void qu_set_master_volume(float volume)
{
    qu.audio.set_master_volume(volume);
}

qu_sound qu_load_sound(char const *path)
{
    libqu_file *file = libqu_fopen(path);

    if (file) {
        int32_t id = qu.audio.load_sound(file);
        libqu_fclose(file);

        return (qu_sound) { id };
    }

    return (qu_sound) { 0 };
}

void qu_delete_sound(qu_sound sound)
{
    qu.audio.delete_sound(sound.id);
}

qu_stream qu_play_sound(qu_sound sound)
{
    return (qu_stream) { qu.audio.play_sound(sound.id) };
}

qu_stream qu_loop_sound(qu_sound sound)
{
    return (qu_stream) { qu.audio.loop_sound(sound.id) };
}

qu_music qu_open_music(char const *path)
{
    libqu_file *file = libqu_fopen(path);

    if (file) {
        int32_t id = qu.audio.open_music(file);
        
        if (id == 0) {
            libqu_fclose(file);
        }

        return (qu_music) { id };
    }

    return (qu_music) { 0 };
}

void qu_close_music(qu_music music)
{
    qu.audio.close_music(music.id);
}

qu_stream qu_play_music(qu_music music)
{
    return (qu_stream) { qu.audio.play_music(music.id) };
}

qu_stream qu_loop_music(qu_music music)
{
    return (qu_stream) { qu.audio.loop_music(music.id) };
}

void qu_pause_stream(qu_stream stream)
{
    qu.audio.pause_stream(stream.id);
}

void qu_unpause_stream(qu_stream stream)
{
    qu.audio.unpause_stream(stream.id);
}

void qu_stop_stream(qu_stream stream)
{
    qu.audio.stop_stream(stream.id);
}

//------------------------------------------------------------------------------
