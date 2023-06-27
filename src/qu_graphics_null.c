//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include "qu.h"

//------------------------------------------------------------------------------

static void initialize(qu_params const *params)
{
    libqu_info("Null graphics module initialized.\n");
}

static void terminate(void)
{
    libqu_info("Null graphics module terminated.\n");
}

static bool is_initialized(void)
{
    return true;
}

static void swap(void)
{
}

static void notify_display_resize(int width, int height)
{
}

//------------------------------------------------------------------------------

static void push_matrix(void)
{
}

static void pop_matrix(void)
{
}

static void translate(float x, float y)
{
}

static void scale(float x, float y)
{
}

static void rotate(float degrees)
{
}

static void clear(qu_color clear_color)
{
}

static void draw_point(float x, float y, qu_color color)
{
}

static void draw_line(float ax, float ay, float bx, float by, qu_color color)
{
}

static void draw_triangle(float ax, float ay, float bx, float by, float cx, float cy, qu_color outline, qu_color fill)
{
}

static void draw_rectangle(float x, float y, float w, float h, qu_color outline, qu_color fill)
{
}

static void draw_circle(float x, float y, float radius, qu_color outline, qu_color fill)
{
}

//------------------------------------------------------------------------------

static int32_t create_texture(int width, int height, int channels)
{
    return 1;
}

static void update_texture(int32_t texture_id, int x, int y, int w, int h, uint8_t const *pixels)
{
}

static int32_t load_texture(libqu_file *file)
{
    return 1;
}

static void delete_texture(int32_t texture_id)
{
}

static void set_texture_smooth(int32_t texture_id, bool smooth)
{
}

static void draw_texture(int32_t texture_id, float x, float y, float w, float h)
{
}

static void draw_subtexture(int32_t texture_id, float x, float y, float w, float h, float rx, float ry, float rw, float rh)
{
}

static void draw_text(int32_t texture_id, qu_color color, float const *data, int count)
{
}

//------------------------------------------------------------------------------

void libqu_construct_null_graphics(libqu_graphics *graphics)
{
    *graphics = (libqu_graphics) {
        .initialize = initialize,
        .terminate = terminate,
        .is_initialized = is_initialized,
        .swap = swap,
        .notify_display_resize = notify_display_resize,
        .push_matrix = push_matrix,
        .pop_matrix = pop_matrix,
        .translate = translate,
        .scale = scale,
        .rotate = rotate,
        .clear = clear,
        .draw_point = draw_point,
        .draw_line = draw_line,
        .draw_triangle = draw_triangle,
        .draw_rectangle = draw_rectangle,
        .draw_circle = draw_circle,
        .create_texture = create_texture,
        .update_texture = update_texture,
        .load_texture = load_texture,
        .delete_texture = delete_texture,
        .set_texture_smooth = set_texture_smooth,
        .draw_texture = draw_texture,
        .draw_subtexture = draw_subtexture,
        .draw_text = draw_text,
    };
}

//------------------------------------------------------------------------------
