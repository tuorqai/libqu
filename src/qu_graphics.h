//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_GRAPHICS_H
#define QU_GRAPHICS_H

//------------------------------------------------------------------------------

#include "libqu.h"
#include "qu_fs.h"

//------------------------------------------------------------------------------

typedef struct libqu_graphics
{
    void (*initialize)(qu_params const *params);
    void (*terminate)(void);
    bool (*is_initialized)(void);
    void (*swap)(void);
    void (*notify_display_resize)(int width, int height);
    qu_vec2i (*conv_cursor)(qu_vec2i position);
    qu_vec2i (*conv_cursor_delta)(qu_vec2i position);

    // API: qu_set_view()
    void (*set_view)(float x, float y, float w, float h, float rotation);

    // API: qu_reset_view()
    void (*reset_view)(void);

    void (*push_matrix)(void);
    void (*pop_matrix)(void);
    void (*translate)(float x, float y);
    void (*scale)(float x, float y);
    void (*rotate)(float degrees);

    // API: qu_clear()
    void (*clear)(qu_color color);

    // API: qu_draw_point()
    void (*draw_point)(float x, float y, qu_color color);

    // API: qu_draw_line()
    void (*draw_line)(float ax, float ay, float bx, float by, qu_color color);

    // API: qu_draw_triangle()
    void (*draw_triangle)(float ax, float ay, float bx, float by, float cx,
        float cy, qu_color outline, qu_color fill);

    // API: qu_draw_rectangle()
    void (*draw_rectangle)(float x, float y, float w, float h, qu_color outline,
        qu_color fill);

    // API: qu_draw_circle()
    void (*draw_circle)(float x, float y, float radius, qu_color outline,
        qu_color fill);

    // API: <none>
    int32_t (*create_texture)(int w, int h, int channels);

    // API: <none>
    void (*update_texture)(int32_t texture_id, int x, int y, int w, int h,
        uint8_t const *pixels);

    // API: qu_load_texture()
    int32_t (*load_texture)(libqu_file *file);

    // API: qu_delete_texture()
    void (*delete_texture)(int32_t texture_id);

    // API: qu_set_texture_smooth()
    void (*set_texture_smooth)(int32_t texture_id, bool smooth);

    // API: qu_draw_texture()
    void (*draw_texture)(int32_t texture_id, float x, float y, float w,
        float h);

    // API: qu_draw_subtexture()
    void (*draw_subtexture)(int32_t texture_id, float x, float y, float w,
        float h, float rx, float ry, float rw, float rh);

    // API: <none>
    void (*draw_text)(int32_t texture_id, qu_color color, float const *data,
        int count);

    // API: qu_create_surface()
    int32_t (*create_surface)(int width, int height);

    // API: qu_delete_surface()
    void (*delete_surface)(int32_t id);

    // API: qu_set_surface()
    void (*set_surface)(int32_t id);

    // API: qu_reset_surface()
    void (*reset_surface)(void);

    // API: qu_draw_surface()
    void (*draw_surface)(int32_t id, float x, float y, float w, float h);
} libqu_graphics;

//------------------------------------------------------------------------------

void libqu_construct_null_graphics(libqu_graphics *graphics);

#if !defined(QU_DISABLE_GL)
void libqu_construct_gl2_graphics(libqu_graphics *graphics);
#endif

#if !defined(QU_DISABLE_GLES2)
void libqu_construct_gles2_graphics(libqu_graphics *graphics);
#endif

//------------------------------------------------------------------------------

#endif // QU_GRAPHICS_H

//------------------------------------------------------------------------------
