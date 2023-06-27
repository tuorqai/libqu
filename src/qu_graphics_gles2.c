
//------------------------------------------------------------------------------

#include <GLES2/gl2.h>

//------------------------------------------------------------------------------
// Adapter macros

#define GL2_SHADER_VERTEX_SRC \
    "attribute vec2 a_position;\n" \
    "attribute vec4 a_color;\n" \
    "attribute vec2 a_texCoord;\n" \
    "varying vec4 v_color;\n" \
    "varying vec2 v_texCoord;\n" \
    "uniform mat4 u_projection;\n" \
    "uniform mat4 u_modelView;\n" \
    "void main()\n" \
    "{\n" \
    "    v_texCoord = a_texCoord;\n" \
    "    v_color = a_color;\n" \
    "    vec4 position = vec4(a_position, 0.0, 1.0);\n" \
    "    gl_Position = u_projection * u_modelView * position;\n" \
    "}\n"

#define GL2_SHADER_SOLID_SRC \
    "precision mediump float;\n" \
    "uniform vec4 u_color;\n" \
    "void main()\n" \
    "{\n" \
    "    gl_FragColor = u_color;\n" \
    "}\n"

#define GL2_SHADER_TEXTURED_SRC \
    "precision mediump float;\n" \
    "varying vec2 v_texCoord;\n" \
    "uniform sampler2D u_texture;\n" \
    "uniform vec4 u_color;\n" \
    "void main()\n" \
    "{\n" \
    "    gl_FragColor = texture2D(u_texture, v_texCoord) * u_color;\n" \
    "}\n"

#define GL2_SHADER_CANVAS_SRC \
    "attribute vec2 a_position;\n" \
    "attribute vec2 a_texCoord;\n" \
    "varying vec2 v_texCoord;\n" \
    "uniform mat4 u_projection;\n" \
    "void main()\n" \
    "{\n" \
    "    v_texCoord = a_texCoord;\n" \
    "    vec4 position = vec4(a_position, 0.0, 1.0);\n" \
    "    gl_Position = u_projection * position;\n" \
    "}\n"

//------------------------------------------------------------------------------
// Shared implementation

#include "qu_graphics_gl2_impl.h"

//------------------------------------------------------------------------------
// Initializer

static void initialize(qu_params const *params)
{
    gl2_initialize(params);
}

static bool is_initialized(void)
{
    return true;
}

//------------------------------------------------------------------------------
// Constructor

void libqu_construct_gles2_graphics(libqu_graphics *graphics)
{
    *graphics = (libqu_graphics) {
        .initialize = initialize,
        .terminate = gl2_terminate,
        .is_initialized = is_initialized,
        .swap = gl2_swap,
        .notify_display_resize = gl2_notify_display_resize,
        .conv_cursor = gl2_conv_cursor,
        .conv_cursor_delta = gl2_conv_cursor_delta,
        .set_view = gl2_set_view,
        .reset_view = gl2_reset_view,
        .push_matrix = gl2_push_matrix,
        .pop_matrix = gl2_pop_matrix,
        .translate = gl2_translate,
        .scale = gl2_scale,
        .rotate = gl2_rotate,
        .clear = gl2_clear,
        .draw_point = gl2_draw_point,
        .draw_line = gl2_draw_line,
        .draw_triangle = gl2_draw_triangle,
        .draw_rectangle = gl2_draw_rectangle,
        .draw_circle = gl2_draw_circle,
        .create_texture = gl2_create_texture,
        .update_texture = gl2_update_texture,
        .load_texture = gl2_load_texture,
        .delete_texture = gl2_delete_texture,
        .set_texture_smooth = gl2_set_texture_smooth,
        .draw_texture = gl2_draw_texture,
        .draw_subtexture = gl2_draw_subtexture,
        .draw_text = gl2_draw_text,
        .create_surface = gl2_create_surface,
        .delete_surface = gl2_delete_surface,
        .set_surface = gl2_set_surface,
        .reset_surface = gl2_reset_surface,
        .draw_surface = gl2_draw_surface,
    };
}
