
//------------------------------------------------------------------------------

#ifndef QU_GRAPHICS_GL2_IMPL_H
#define QU_GRAPHICS_GL2_IMPL_H

//------------------------------------------------------------------------------

#include <math.h>
#include <string.h>

#include "qu_array.h"
#include "qu_gateway.h"
#include "qu_graphics.h"
#include "qu_halt.h"
#include "qu_image.h"
#include "qu_log.h"
#include "qu_math.h"
#include "qu_util.h"

//------------------------------------------------------------------------------

#define MAX_MATRICES                    (32)

//------------------------------------------------------------------------------

enum
{
    VERTEX_ATTRIB_POSITION,
    VERTEX_ATTRIB_COLOR,
    VERTEX_ATTRIB_TEXCOORD,
    NUM_VERTEX_ATTRIBS,
};

enum
{
    VERTEX_FORMAT_SOLID,
    VERTEX_FORMAT_TEXTURED,
    NUM_VERTEX_FORMATS,
};

enum
{
    SHADER_VERTEX,
    SHADER_SOLID,
    SHADER_TEXTURED,
    SHADER_CANVAS,
    NUM_SHADERS,
};

enum
{
    PROGRAM_SHAPE,
    PROGRAM_TEXTURE,
    PROGRAM_CANVAS,
    NUM_PROGRAMS,
};

enum
{
    UNIFORM_PROJECTION,
    UNIFORM_MODELVIEW,
    UNIFORM_COLOR,
    NUM_UNIFORMS,
};

enum
{
    COMMAND_NONE,
    COMMAND_CLEAR,
    COMMAND_DRAW,
    COMMAND_SET_SURFACE,
    COMMAND_RESET_SURFACE,
    COMMAND_SET_VIEW,
    COMMAND_RESET_VIEW,
    COMMAND_PUSH_MATRIX,
    COMMAND_POP_MATRIX,
    COMMAND_TRANSLATE,
    COMMAND_SCALE,
    COMMAND_ROTATE,
    COMMAND_RESIZE,
};

typedef struct
{
    GLenum type;
    GLchar *ident;
    GLchar *source;
} gl2_shader_desc;

typedef struct
{
    GLchar *ident;
    int vs;
    int fs;
} gl2_program_desc;

typedef struct
{
    GLuint handle;
    GLuint width;
    GLuint height;

    GLint channels;
    GLenum format;
} gl2_texture;

typedef struct
{
    GLuint handle;
    GLuint depth;
    int32_t color_id;
    int width;
    int height;
} gl2_surface;

typedef struct
{
    GLuint handle;
    GLint uniform_locations[NUM_UNIFORMS];
    uint32_t dirty;
} gl2_program;

typedef struct
{
    int type;

    union
    {
        struct
        {
            qu_color color;
        } clear;

        struct
        {
            qu_color color;
            int32_t texture_id;
            int program;
            int format;
            int mode;
            int first;
            int count;
        } draw;

        struct
        {
            int32_t id;
        } surface;

        struct
        {
            float x;
            float y;
            float w;
            float h;
            float r;
        } view;

        struct
        {
            int w;
            int h;
        } size;
    };
} gl2_command;

typedef struct
{
    gl2_command *array;
    unsigned int size;
    unsigned int capacity;
} gl2_command_buffer;

typedef struct
{
    float *array;
    unsigned int size;
    unsigned int capacity;

    GLuint vbo;
    GLuint vbo_size;
} gl2_vertex_buffer;

typedef struct
{
    bool use_canvas;

    int display_width;          // current display width
    int display_height;         // current display height
    float display_aspect;       // current display aspect ratio

    int canvas_id;              // canvas surface id
    int canvas_width;           // current canvas width
    int canvas_height;          // current canvas height
    float canvas_aspect;        // current canvas aspect ratio

    float canvas_ax;            // calculated canvas left-most point
    float canvas_ay;            // calculated canvas top-most point
    float canvas_bx;            // calculated canvas right-most point
    float canvas_by;            // calculated canvas bottom-most point

    int texture_id;             // currently bound texture
    int surface_id;             // currently active framebuffer
    int program;                // currently used program
    int vertex_format;          // current vertex format
    qu_color clear_color;       // current clear color
    qu_color draw_color;        // current draw color
    float draw_color_f[4];

    float projection[16];
    float matrix[MAX_MATRICES][16];
    int current_matrix;
} gl2_state;

//------------------------------------------------------------------------------

static char const *s_vertex_attrib_names[NUM_VERTEX_ATTRIBS] = {
    [VERTEX_ATTRIB_POSITION]        = "a_position",
    [VERTEX_ATTRIB_COLOR]           = "a_color",
    [VERTEX_ATTRIB_TEXCOORD]        = "a_texCoord",
};

static int s_vertex_attrib_sizes[NUM_VERTEX_ATTRIBS] = {
    [VERTEX_ATTRIB_POSITION]        = 2,
    [VERTEX_ATTRIB_COLOR]           = 4,
    [VERTEX_ATTRIB_TEXCOORD]        = 2,
};

static int s_vertex_format_masks[NUM_VERTEX_FORMATS] = {
    [VERTEX_FORMAT_SOLID]           = 0x01,
    [VERTEX_FORMAT_TEXTURED]        = 0x05,
};

static gl2_shader_desc s_shaders[NUM_SHADERS] = {
    { GL_VERTEX_SHADER, "SHADER_VERTEX", GL2_SHADER_VERTEX_SRC },
    { GL_FRAGMENT_SHADER, "SHADER_SOLID", GL2_SHADER_SOLID_SRC },
    { GL_FRAGMENT_SHADER, "SHADER_TEXTURED", GL2_SHADER_TEXTURED_SRC },
    { GL_VERTEX_SHADER, "SHADER_CANVAS", GL2_SHADER_CANVAS_SRC },
};

static gl2_program_desc s_programs[NUM_PROGRAMS] = {
    { "PROGRAM_SHAPE", SHADER_VERTEX, SHADER_SOLID },
    { "PROGRAM_TEXTURE", SHADER_VERTEX, SHADER_TEXTURED },
    { "PROGRAM_CANVAS", SHADER_CANVAS, SHADER_TEXTURED },
};

static char const *s_uniform_names[NUM_UNIFORMS] = {
    "u_projection",
    "u_modelView",
    "u_color",
};

//------------------------------------------------------------------------------

static gl2_state            g_state;
static gl2_command_buffer   g_command_buffer;
static gl2_vertex_buffer    g_vertex_buffers[NUM_VERTEX_FORMATS];
static libqu_array          *g_textures;
static libqu_array          *g_surfaces;
static gl2_program          g_programs[NUM_PROGRAMS];

//------------------------------------------------------------------------------

static void unpack_color(qu_color c, float *a)
{
    a[0] = ((c >> 16) & 255) / 255.f;
    a[1] = ((c >> 8) & 255) / 255.f;
    a[2] = ((c >> 0) & 255) / 255.f;
    a[3] = ((c >> 24) & 255) / 255.f;
}

static GLenum texture_format(int channels)
{
    switch (channels) {
    case 1:
        return GL_LUMINANCE;
    case 2:
        return GL_LUMINANCE_ALPHA;
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    default:
        return GL_INVALID_ENUM;
    }
}

static void texture_dtor(void *data)
{
    gl2_texture *texture = data;
    glDeleteTextures(1, &texture->handle);
    // libqu_info("Deleted texture 0x%08x.\n", texture->id);
}

static void surface_dtor(void *data)
{
    gl2_surface *surface = data;

    libqu_array_remove(g_textures, surface->color_id);

    glDeleteFramebuffers(1, &surface->handle);
    glDeleteRenderbuffers(1, &surface->depth);
}

static GLuint load_shader(gl2_shader_desc *desc)
{
    GLuint shader = glCreateShader(desc->type);
    glShaderSource(shader, 1, (GLchar const *const *) &desc->source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char buffer[256];
        glGetShaderInfoLog(shader, 256, NULL, buffer);
        glDeleteShader(shader);

        libqu_error("Failed to compile GLSL shader %s. Reason:\n%s\n",
                    desc->ident, buffer);

        return 0;
    }

    libqu_info("Shader %s is compiled successfully.\n", desc->ident);

    return shader;
}

static GLuint build_program(char const *ident, GLuint vs, GLuint fs)
{
    GLuint program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    for (int j = 0; j < NUM_VERTEX_ATTRIBS; j++) {
        glBindAttribLocation(program, j, s_vertex_attrib_names[j]);
    }

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        char buffer[256];
        glGetProgramInfoLog(program, 256, NULL, buffer);
        glDeleteProgram(program);

        libqu_error("Failed to link GLSL program %s: %s\n", ident, buffer);

        return 0;
    }

    libqu_info("Shader program %s is compiled successfully.\n", ident);

    return program;
}

static void make_circle(float x, float y, float radius, float *data, int num)
{
    float angle = QU_DEG2RAD(360.f / num);

    for (int i = 0; i < num; i++) {
        data[2 * i + 0] = x + (radius * cosf(i * angle));
        data[2 * i + 1] = y + (radius * sinf(i * angle));
    }
}

//------------------------------------------------------------------------------

static void update_canvas_coords(int w_display, int h_display)
{
    struct surface *canvas = libqu_array_get(g_surfaces, g_state.canvas_id);

    float ard = g_state.display_aspect;
    float arc = g_state.canvas_aspect;

    if (ard > arc) {
        g_state.canvas_ax = (w_display / 2.f) - ((arc / ard) * w_display / 2.f);
        g_state.canvas_ay = 0.f;
        g_state.canvas_bx = (w_display / 2.f) + ((arc / ard) * w_display / 2.f);
        g_state.canvas_by = h_display;
    } else {
        g_state.canvas_ax = 0.f;
        g_state.canvas_ay = (h_display / 2.f) - ((ard / arc) * h_display / 2.f);
        g_state.canvas_bx = w_display;
        g_state.canvas_by = (h_display / 2.f) + ((ard / arc) * h_display / 2.f);
    }
}

//------------------------------------------------------------------------------

static void upload_uniform(int uniform, GLuint location)
{
    switch (uniform) {
    case UNIFORM_PROJECTION:
        glUniformMatrix4fv(location, 1, GL_FALSE, g_state.projection);
        break;
    case UNIFORM_MODELVIEW:
        glUniformMatrix4fv(location, 1, GL_FALSE, g_state.matrix[g_state.current_matrix]);
        break;
    case UNIFORM_COLOR:
        glUniform4fv(location, 1, g_state.draw_color_f);
        break;
    default:
        break;
    }
}

static void update_projection(float x, float y, float w, float h, float rot)
{
    float l = x - (w / 2.f);
    float r = x + (w / 2.f);
    float b = y + (h / 2.f);
    float t = y - (h / 2.f);

    libqu_mat4_ortho(g_state.projection, l, r, b, t);

    if (rot != 0.f) {
        libqu_mat4_translate(g_state.projection, x, y, 0.f);
        libqu_mat4_rotate(g_state.projection, QU_DEG2RAD(rot), 0.f, 0.f, 1.f);
        libqu_mat4_translate(g_state.projection, -x, -y, 0.f);
    }

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        g_programs[i].dirty |= (1 << UNIFORM_PROJECTION);
    }
}

static void update_model_view(void)
{
    for (int i = 0; i < NUM_PROGRAMS; i++) {
        g_programs[i].dirty |= (1 << UNIFORM_MODELVIEW);
    }
}

static void update_draw_color(qu_color color)
{
    if (g_state.draw_color == color) {
        return;
    }

    unpack_color(color, g_state.draw_color_f);

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        g_programs[i].dirty |= (1 << UNIFORM_COLOR);
    }

    g_state.draw_color = color;
}

//------------------------------------------------------------------------------

static void update_clear_color(qu_color color)
{
    if (g_state.clear_color == color) {
        return;
    }

    float c[4];

    unpack_color(color, c);
    glClearColor(c[0], c[1], c[2], c[3]);

    g_state.clear_color = color;
}

static void update_program(int program)
{
    if (g_state.program == program && !g_programs[program].dirty) {
        return;
    }

    glUseProgram(g_programs[program].handle);

    if (g_programs[program].dirty) {
        for (int i = 0; i < NUM_UNIFORMS; i++) {
            if (g_programs[program].dirty & (1 << i)) {
                upload_uniform(i, g_programs[program].uniform_locations[i]);
            }
        }
    }

    g_programs[program].dirty = 0;
    g_state.program = program;
}

static void update_vertex_format(int format)
{
    if (g_state.vertex_format == format) {
        return;
    }

    int mask = s_vertex_format_masks[format];

    glBindBuffer(GL_ARRAY_BUFFER, g_vertex_buffers[format].vbo);

    GLsizei stride = 0;

    for (int i = 0; i < NUM_VERTEX_ATTRIBS; i++) {
        if (mask & (1 << i)) {
            glEnableVertexAttribArray(i);
            stride += sizeof(GLfloat) * s_vertex_attrib_sizes[i];
        } else {
            glDisableVertexAttribArray(i);
        }
    }

    GLsizei offset = 0;

    for (int i = 0; i < NUM_VERTEX_ATTRIBS; i++) {
        if (mask & (1 << i)) {
            glVertexAttribPointer(i, s_vertex_attrib_sizes[i], GL_FLOAT,
                                  GL_FALSE, stride, (void *) offset);
            offset += sizeof(GLfloat) * s_vertex_attrib_sizes[i];
        }
    }

    g_state.vertex_format = format;
}

static void update_texture(int32_t id)
{
    if (g_state.texture_id == id) {
        return;
    }

    GLuint handle;

    if (id == 0) {
        handle = 0;
    } else {
        gl2_texture *texture = libqu_array_get(g_textures, id);

        if (!texture) {
            return;
        }

        handle = texture->handle;
    }

    glBindTexture(GL_TEXTURE_2D, handle);
    g_state.texture_id = id;
}

static void update_surface(int32_t id)
{
    if (g_state.surface_id == id) {
        return;
    }

    GLuint handle;
    GLsizei width, height;

    if (id == 0) {
        handle = 0;
        width = g_state.display_width;
        height = g_state.display_height;
    } else {
        gl2_surface *surface = libqu_array_get(g_surfaces, id);

        if (!surface) {
            return;
        }

        handle = surface->handle;
        width = surface->width;
        height = surface->height;
    }

    // Restore view
    update_projection(width / 2.f, height / 2.f, width, height, 0.f);

    // Restore transformation stack
    g_state.current_matrix = 0;
    libqu_mat4_identity(g_state.matrix[0]);
    update_model_view();

    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glViewport(0, 0, width, height);

    g_state.surface_id = id;
}

//------------------------------------------------------------------------------
// Commands

static void exec_clear(qu_color color)
{
    update_clear_color(color);

    glClear(GL_COLOR_BUFFER_BIT);
}

static void exec_draw(qu_color color, int32_t texture, int program, int format,
                      GLenum mode, GLint first, GLsizei count)
{
    update_draw_color(color);
    update_texture(texture);
    update_program(program);
    update_vertex_format(format);

    glDrawArrays(mode, first, count);
}

static void exec_set_surface(int32_t id)
{
    update_surface(id);
}

static void exec_reset_surface(void)
{
    if (g_state.use_canvas) {
        update_surface(g_state.canvas_id);
    } else {
        update_surface(0);
    }
}

static void exec_set_view(float x, float y, float w, float h, float rotation)
{
    update_projection(x, y, w, h, rotation);
}

static void exec_reset_view(void)
{
    GLsizei width, height;

    if (g_state.surface_id == 0) {
        width = g_state.display_width;
        height = g_state.display_height;
    } else {
        gl2_surface *surface = libqu_array_get(g_surfaces, g_state.surface_id);

        if (!surface) {
            return;
        }

        width = surface->width;
        height = surface->height;
    }

    update_projection(width / 2.f, height / 2.f, width, height, 0.f);
}

static void exec_push_matrix(void)
{
    if (g_state.current_matrix == (MAX_MATRICES - 1)) {
        libqu_warning("Can't qu_push_matrix(): limit of %d matrices reached.\n", MAX_MATRICES);
        return;
    }

    float *next = g_state.matrix[g_state.current_matrix + 1];
    float *current = g_state.matrix[g_state.current_matrix];

    libqu_mat4_copy(next, current);

    g_state.current_matrix++;
}

static void exec_pop_matrix(void)
{
    if (g_state.current_matrix == 0) {
        libqu_warning("Can't qu_pop_matrix(): already at the first matrix.\n");
        return;
    }

    g_state.current_matrix--;
}

static void exec_translate(float x, float y)
{
    float *matrix = g_state.matrix[g_state.current_matrix];
    libqu_mat4_translate(matrix, x, y, 0.f);
    update_model_view();
}

static void exec_scale(float x, float y)
{
    float *matrix = g_state.matrix[g_state.current_matrix];
    libqu_mat4_scale(matrix, x, y, 1.f);
    update_model_view();
}

static void exec_rotate(float degrees)
{
    float *matrix = g_state.matrix[g_state.current_matrix];
    libqu_mat4_rotate(matrix, QU_DEG2RAD(degrees), 0.f, 0.f, 1.f);
    update_model_view();
}

static void exec_resize(int width, int height)
{
    g_state.display_width = width;
    g_state.display_height = height;
    g_state.display_aspect = width / (float) height;

    if (g_state.use_canvas) {
        update_canvas_coords(width, height);
    }

    if (g_state.surface_id == 0) {
        glViewport(0, 0, width, height);
        update_projection(width / 2.f, height / 2.f, width, height, 0.f);
    }
}

//------------------------------------------------------------------------------
// Command buffer

static void append_command(gl2_command const *command)
{
    gl2_command_buffer *buffer = &g_command_buffer;

    if (buffer->size == buffer->capacity) {
        unsigned int next_capacity = buffer->capacity * 2;

        if (!next_capacity) {
            next_capacity = 256;
        }

        gl2_command *next_array =
            realloc(buffer->array, sizeof(gl2_command) * next_capacity);

        if (!next_array) {
            return;
        }

        buffer->array = next_array;
        buffer->capacity = next_capacity;
    }

    memcpy(&buffer->array[buffer->size++], command, sizeof(gl2_command));
}

static void execute_command(gl2_command *command)
{
    switch (command->type) {
    case COMMAND_CLEAR:
        exec_clear(command->clear.color);
        break;
    case COMMAND_DRAW:
        exec_draw(command->draw.color, command->draw.texture_id,
                  command->draw.program, command->draw.format,
                  command->draw.mode, command->draw.first, command->draw.count);
        break;
    case COMMAND_SET_SURFACE:
        exec_set_surface(command->surface.id);
        break;
    case COMMAND_RESET_SURFACE:
        exec_reset_surface();
        break;
    case COMMAND_SET_VIEW:
        exec_set_view(command->view.x, command->view.y,
                      command->view.w, command->view.h,
                      command->view.r);
        break;
    case COMMAND_RESET_VIEW:
        exec_reset_view();
        break;
    case COMMAND_PUSH_MATRIX:
        exec_push_matrix();
        break;
    case COMMAND_POP_MATRIX:
        exec_pop_matrix();
        break;
    case COMMAND_TRANSLATE:
        exec_translate(command->view.x, command->view.y);
        break;
    case COMMAND_SCALE:
        exec_scale(command->view.x, command->view.y);
        break;
    case COMMAND_ROTATE:
        exec_rotate(command->view.r);
        break;
    case COMMAND_RESIZE:
        exec_resize(command->size.w, command->size.h);
        break;
    default:
        break;
    }
}

//------------------------------------------------------------------------------
// Vertex buffer

static int append_vertex_data(int format, float const *data, int size)
{
    gl2_vertex_buffer *buffer = &g_vertex_buffers[format];

    unsigned int required = buffer->size + size;

    if (required > buffer->capacity) {
        unsigned int next_capacity = buffer->capacity;

        if (next_capacity == 0) {
            next_capacity = 256;
        }

        while (next_capacity < required) {
            next_capacity *= 2;
        }

        float *next_array = realloc(buffer->array, sizeof(float) * next_capacity);

        if (!next_array) {
            return 0;
        }

        libqu_debug("GLES 2.0: grow vertex array %d [%d -> %d]\n", format,
                    buffer->capacity, next_capacity);

        buffer->array = next_array;
        buffer->capacity = next_capacity;
    }

    memcpy(buffer->array + buffer->size, data, sizeof(float) * size);

    int offset = buffer->size;
    buffer->size += size;

    return offset;
}

//------------------------------------------------------------------------------
// Views

static void gl2_set_view(float x, float y, float w, float h, float rotation)
{
    append_command(&(gl2_command) {
        .type = COMMAND_SET_VIEW,
        .view = {
            .x = x,
            .y = y,
            .w = w,
            .h = h,
            .r = rotation,
        },
    });
}

static void gl2_reset_view(void)
{
    append_command(&(gl2_command) {
        .type = COMMAND_RESET_VIEW,
    });
}

//------------------------------------------------------------------------------
// Transformation

static void gl2_push_matrix(void)
{
    append_command(&(gl2_command) {
        .type = COMMAND_PUSH_MATRIX,
    });
}

static void gl2_pop_matrix(void)
{
    append_command(&(gl2_command) {
        .type = COMMAND_POP_MATRIX,
    });
}

static void gl2_translate(float x, float y)
{
    append_command(&(gl2_command) {
        .type = COMMAND_TRANSLATE,
        .view.x = x,
        .view.y = y,
    });
}

static void gl2_scale(float x, float y)
{
    append_command(&(gl2_command) {
        .type = COMMAND_SCALE,
        .view.x = x,
        .view.y = y,
    });
}

static void gl2_rotate(float degrees)
{
    append_command(&(gl2_command) {
        .type = COMMAND_ROTATE,
        .view.r = degrees,
    });
}

//------------------------------------------------------------------------------
// Primitives

static void gl2_clear(qu_color color)
{
    append_command(&(gl2_command) {
        .type = COMMAND_CLEAR,
        .clear = {
            .color = color,
        },
    });
}

static void gl2_draw_point(float x, float y, qu_color color)
{
    float vertices[] = {
        x, y,
    };

    append_command(&(gl2_command) {
        .type = COMMAND_DRAW,
        .draw = {
            .color = color,
            .program = PROGRAM_SHAPE,
            .format = VERTEX_FORMAT_SOLID,
            .mode = GL_POINTS,
            .first = append_vertex_data(VERTEX_FORMAT_SOLID, vertices, 2) / 2,
            .count = 2,
        },
    });
}

static void gl2_draw_line(float ax, float ay, float bx, float by, qu_color color)
{
    float vertices[] = {
        ax, ay,
        bx, by,
    };

    append_command(&(gl2_command) {
        .type = COMMAND_DRAW,
        .draw = {
            .color = color,
            .program = PROGRAM_SHAPE,
            .format = VERTEX_FORMAT_SOLID,
            .mode = GL_LINES,
            .first = append_vertex_data(VERTEX_FORMAT_SOLID, vertices, 4) / 2,
            .count = 2,
        },
    });
}

static void gl2_draw_triangle(float ax, float ay, float bx, float by,
                              float cx, float cy, qu_color outline, qu_color fill)
{
    int fill_alpha = (fill >> 24) & 255;
    int outline_alpha = (outline >> 24) & 255;

    float vertices[] = {
        ax, ay,
        bx, by,
        cx, cy,
    };

    int first = append_vertex_data(VERTEX_FORMAT_SOLID, vertices, 6) / 2;

    if (fill_alpha > 0) {
        append_command(&(gl2_command) {
            .type = COMMAND_DRAW,
            .draw = {
                .color = fill,
                .program = PROGRAM_SHAPE,
                .format = VERTEX_FORMAT_SOLID,
                .mode = GL_TRIANGLE_FAN,
                .first = first,
                .count = 3,
            },
        });
    }

    if (outline_alpha > 0) {
        append_command(&(gl2_command) {
            .type = COMMAND_DRAW,
            .draw = {
                .color = outline,
                .program = PROGRAM_SHAPE,
                .format = VERTEX_FORMAT_SOLID,
                .mode = GL_LINE_LOOP,
                .first = first,
                .count = 3,
            },
        });
    }
}

static void gl2_draw_rectangle(float x, float y, float w, float h,
                               qu_color outline, qu_color fill)
{
    int fill_alpha = (fill >> 24) & 255;
    int outline_alpha = (outline >> 24) & 255;

    float vertices[] = {
        x, y,
        x + w, y,
        x + w, y + h,
        x, y + h,
    };

    int first = append_vertex_data(VERTEX_FORMAT_SOLID, vertices, 8) / 2;

    if (fill_alpha > 0) {
        append_command(&(gl2_command) {
            .type = COMMAND_DRAW,
            .draw = {
                .color = fill,
                .program = PROGRAM_SHAPE,
                .format = VERTEX_FORMAT_SOLID,
                .mode = GL_TRIANGLE_FAN,
                .first = first,
                .count = 4,
            },
        });
    }

    if (outline_alpha > 0) {
        append_command(&(gl2_command) {
            .type = COMMAND_DRAW,
            .draw = {
                .color = outline,
                .program = PROGRAM_SHAPE,
                .format = VERTEX_FORMAT_SOLID,
                .mode = GL_LINE_LOOP,
                .first = first,
                .count = 4,
            },
        });
    }
}

static void gl2_draw_circle(float x, float y, float radius, qu_color outline, qu_color fill)
{
    int fill_alpha = (fill >> 24) & 255;
    int outline_alpha = (outline >> 24) & 255;

    float vertices[64];
    make_circle(x, y, radius, vertices, 32);

    int first = append_vertex_data(VERTEX_FORMAT_SOLID, vertices, 64) / 2;

    if (fill_alpha > 0) {
        append_command(&(gl2_command) {
            .type = COMMAND_DRAW,
            .draw = {
                .color = fill,
                .program = PROGRAM_SHAPE,
                .format = VERTEX_FORMAT_SOLID,
                .mode = GL_TRIANGLE_FAN,
                .first = first,
                .count = 32,
            },
        });
    }

    if (outline_alpha > 0) {
        append_command(&(gl2_command) {
            .type = COMMAND_DRAW,
            .draw = {
                .color = outline,
                .program = PROGRAM_SHAPE,
                .format = VERTEX_FORMAT_SOLID,
                .mode = GL_LINE_LOOP,
                .first = first,
                .count = 32,
            },
        });
    }
}

//------------------------------------------------------------------------------
// Textures

static int32_t gl2_create_texture(int width, int height, int channels)
{
    if ((width < 0) || (height < 0)) {
        return 0;
    }

    if ((channels < 1) || (channels > 4)) {
        return 0;
    }

    GLenum format = texture_format(channels);

    if (format == GL_INVALID_ENUM) {
        return 0;
    }

    gl2_texture texture = {0};

    glGenTextures(1, &texture.handle);

    glBindTexture(GL_TEXTURE_2D, texture.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
                 0, format, GL_UNSIGNED_BYTE, NULL);

    texture.width = width;
    texture.height = height;
    texture.channels = channels;
    texture.format = format;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    g_state.texture_id = libqu_array_add(g_textures, &texture);

    if (g_state.texture_id > 0) {
        libqu_info("Created texture 0x%08x.\n", g_state.texture_id);
    }

    return g_state.texture_id;
}

static void gl2_update_texture(int32_t texture_id, int x, int y, int w, int h,
                               uint8_t const *pixels)
{
    gl2_texture *texture = libqu_array_get(g_textures, texture_id);

    if (!texture) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, texture->handle);

    if (x == 0 && y == 0 && w == -1 && h == -1) {
        glTexImage2D(GL_TEXTURE_2D, 0, texture->format,
                     texture->width, texture->height, 0,
                     texture->format, GL_UNSIGNED_BYTE, pixels);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
                        texture->format, GL_UNSIGNED_BYTE, pixels);
    }

    g_state.texture_id = texture_id;
}

static int32_t gl2_load_texture(libqu_file *file)
{
    libqu_image *image = libqu_load_image(file);
    libqu_fclose(file);

    if (!image) {
        return 0;
    }

    GLenum format = texture_format(image->channels);

    if (format == GL_INVALID_ENUM) {
        libqu_delete_image(image);
        return 0;
    }

    gl2_texture texture = {0};

    glGenTextures(1, &texture.handle);

    glBindTexture(GL_TEXTURE_2D, texture.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, format, image->width, image->height,
                 0, format, GL_UNSIGNED_BYTE, image->pixels);

    texture.width = image->width;
    texture.height = image->height;
    texture.channels = image->channels;
    texture.format = format;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#ifdef __EMSCRIPTEN__
    // I don't know what's going on, but without these parameters
    // textures are rendered as black in WebGL

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif

    g_state.texture_id = libqu_array_add(g_textures, &texture);

    if (g_state.texture_id > 0) {
        libqu_info("Loaded texture 0x%08x.\n", g_state.texture_id);
    }

    libqu_delete_image(image);

    return g_state.texture_id;
}

static void gl2_delete_texture(int32_t texture_id)
{
    gl2_texture *texture = libqu_array_get(g_textures, texture_id);

    if (!texture) {
        return;
    }

    libqu_array_remove(g_textures, texture_id);
}

static void gl2_set_texture_smooth(int32_t texture_id, bool smooth)
{
    gl2_texture *texture = libqu_array_get(g_textures, texture_id);

    if (!texture) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, texture->handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);

    g_state.texture_id = texture_id;
}

static void gl2_draw_texture(int32_t texture_id, float x, float y, float w, float h)
{
    float vertices[] = {
        x,      y,      0.f,    0.f,
        x + w,  y,      1.f,    0.f,
        x + w,  y + h,  1.f,    1.f,
        x,      y + h,  0.f,    1.f,
    };

    append_command(&(gl2_command) {
        .type = COMMAND_DRAW,
        .draw = {
            .color = 0xffffffff,
            .texture_id = texture_id,
            .program = PROGRAM_TEXTURE,
            .format = VERTEX_FORMAT_TEXTURED,
            .mode = GL_TRIANGLE_FAN,
            .first = append_vertex_data(VERTEX_FORMAT_TEXTURED, vertices, 16) / 4,
            .count = 4,
        },
    });
}

static void gl2_draw_subtexture(int32_t texture_id, float x, float y, float w,
                                float h, float rx, float ry, float rw, float rh)
{
    gl2_texture *texture = libqu_array_get(g_textures, texture_id);

    if (!texture) {
        return;
    }

    float s = rx / texture->width;
    float t = ry / texture->height;
    float u = rw / texture->width;
    float v = rh / texture->height;

    float vertices[] = {
        x,      y,      s,      t,
        x + w,  y,      s + u,  t,
        x + w,  y + h,  s + u,  t + v,
        x,      y + h,  s,      t + v,
    };

    append_command(&(gl2_command) {
        .type = COMMAND_DRAW,
        .draw = {
            .color = 0xffffffff,
            .texture_id = texture_id,
            .program = PROGRAM_TEXTURE,
            .format = VERTEX_FORMAT_TEXTURED,
            .mode = GL_TRIANGLE_FAN,
            .first = append_vertex_data(VERTEX_FORMAT_TEXTURED, vertices, 16) / 4,
            .count = 4,
        },
    });
}

//------------------------------------------------------------------------------
// Fonts

static void gl2_draw_text(int32_t texture_id, qu_color color, float const *data, int count)
{
    append_command(&(gl2_command) {
        .type = COMMAND_DRAW,
        .draw = {
            .color = color,
            .texture_id = texture_id,
            .program = PROGRAM_TEXTURE,
            .format = VERTEX_FORMAT_TEXTURED,
            .mode = GL_TRIANGLES,
            .first = append_vertex_data(VERTEX_FORMAT_TEXTURED, data, count * 4) / 4,
            .count = count,
        },
    });
}

//------------------------------------------------------------------------------
// Surfaces

static int32_t gl2_create_surface(int width, int height)
{
    if (width < 0 || height < 0) {
        return 0;
    }

    gl2_surface surface = {0};

    glGenFramebuffers(1, &surface.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, surface.handle);

    glGenRenderbuffers(1, &surface.depth);
    glBindRenderbuffer(GL_RENDERBUFFER, surface.depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, surface.depth);

    surface.color_id = gl2_create_texture(width, height, 4);

    if (surface.color_id == 0) {
        glDeleteFramebuffers(1, &surface.handle);
        return 0;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gl2_texture *texture = libqu_array_get(g_textures, surface.color_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture->handle, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        libqu_error("Failed to create OpenGL framebuffer.\n");
        return 0;
    }

    surface.width = width;
    surface.height = height;

    g_state.surface_id = libqu_array_add(g_surfaces, &surface);

    if (g_state.surface_id == 0) {
        libqu_error("Failed to create surface: insufficient memory.\n");
        return 0;
    }

    return g_state.surface_id;
}

static void gl2_delete_surface(int32_t id)
{
    libqu_array_remove(g_surfaces, id);
}

static void gl2_set_surface(int32_t id)
{
    append_command(&(gl2_command) {
        .type = COMMAND_SET_SURFACE,
        .surface.id = id,
    });
}

static void gl2_reset_surface(void)
{
    append_command(&(gl2_command) {
        .type = COMMAND_RESET_SURFACE,
    });
}

static void gl2_draw_surface(int32_t id, float x, float y, float w, float h)
{
    gl2_surface *surface = libqu_array_get(g_surfaces, id);

    if (!surface) {
        return;
    }

    float vertices[] = {
        x,      y,      0.f,    1.f,
        x + w,  y,      1.f,    1.f,
        x + w,  y + h,  1.f,    0.f,
        x,      y + h,  0.f,    0.f,
    };

    append_command(&(gl2_command) {
        .type = COMMAND_DRAW,
        .draw = {
            .color = 0xffffffff,
            .texture_id = surface->color_id,
            .program = PROGRAM_TEXTURE,
            .format = VERTEX_FORMAT_TEXTURED,
            .mode = GL_TRIANGLE_FAN,
            .first = append_vertex_data(VERTEX_FORMAT_TEXTURED, vertices, 16) / 4,
            .count = 4,
        },
    });
}

//------------------------------------------------------------------------------

static void gl2_initialize(qu_params const *params)
{
    g_textures = libqu_create_array(sizeof(gl2_texture), texture_dtor);
    g_surfaces = libqu_create_array(sizeof(gl2_surface), surface_dtor);

    if (!g_textures || !g_surfaces) {
        libqu_halt("Failed to initialize OpenGL");
    }

    GLuint shaders[NUM_SHADERS];

    for (int i = 0; i < NUM_SHADERS; i++) {
        shaders[i] = load_shader(&s_shaders[i]);

        if (!shaders[i]) {
            libqu_halt("Failed to initialize OpenGL");
        }
    }

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        g_programs[i].handle =
            build_program(s_programs[i].ident, shaders[s_programs[i].vs],
                          shaders[s_programs[i].fs]);

        for (int j = 0; j < NUM_UNIFORMS; j++) {
            g_programs[i].uniform_locations[j] =
                glGetUniformLocation(g_programs[i].handle, s_uniform_names[j]);
        }

        g_programs[i].dirty = (uint32_t) -1;
    }

    for (int i = 0; i < NUM_SHADERS; i++) {
        glDeleteShader(shaders[i]);
    }

    for (int i = 0; i < NUM_VERTEX_FORMATS; i++) {
        glGenBuffers(1, &g_vertex_buffers[i].vbo);
    }

    g_state.use_canvas = params->enable_canvas;

    g_state.display_width = params->display_width;
    g_state.display_height = params->display_height;
    g_state.display_aspect = params->display_width / (float) params->display_height;

    if (g_state.use_canvas) {
        g_state.canvas_width = params->canvas_width;
        g_state.canvas_height = params->canvas_height;
        g_state.canvas_aspect = params->canvas_width / (float) params->canvas_height;

        g_state.canvas_id = gl2_create_surface(g_state.canvas_width, g_state.canvas_height);

        if (!g_state.canvas_id) {
            libqu_halt("Failed to initialize default framebuffer.\n");
        }

        update_canvas_coords(g_state.display_width, g_state.display_height);
    }

    append_command(&(gl2_command) {
        .type = COMMAND_RESET_SURFACE,
    });

    g_state.texture_id = -1;
    g_state.surface_id = -1;
    g_state.program = -1;
    g_state.vertex_format = -1;
    g_state.clear_color = 0;
    g_state.draw_color = 0;

    libqu_mat4_ortho(g_state.projection, 0.f,
                     params->display_width, params->display_height, 0.f);

    g_state.current_matrix = 0;
    libqu_mat4_identity(g_state.matrix[0]);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    libqu_info("OpenGL 2.1 graphics module initialized.\n");
    libqu_info("OpenGL vendor: %s\n", glGetString(GL_VENDOR));
    libqu_info("OpenGL version: %s\n", glGetString(GL_VERSION));
    libqu_info("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
}

static void gl2_terminate(void)
{
    libqu_destroy_array(g_surfaces);
    libqu_destroy_array(g_textures);
    free(g_command_buffer.array);

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        glDeleteProgram(g_programs[i].handle);
    }

    libqu_info("OpenGL 2.1 graphics module terminated.\n");
}

static void gl2_swap(void)
{
    // If using canvas, then draw it in the default framebuffer
    if (g_state.use_canvas) {
        append_command(&(gl2_command) {
            .type = COMMAND_SET_SURFACE,
            .surface.id = 0,
        });

        append_command(&(gl2_command) {
            .type = COMMAND_CLEAR,
            .clear.color = 0xff000000,
        });

        gl2_surface *canvas = libqu_array_get(g_surfaces, g_state.canvas_id);

        float vertices[] = {
            g_state.canvas_ax, g_state.canvas_ay, 0.f, 1.f,
            g_state.canvas_bx, g_state.canvas_ay, 1.f, 1.f,
            g_state.canvas_bx, g_state.canvas_by, 1.f, 0.f,
            g_state.canvas_ax, g_state.canvas_by, 0.f, 0.f,
        };

        append_command(&(gl2_command) {
            .type = COMMAND_DRAW,
            .draw = {
                .color = 0xffffffff,
                .texture_id = canvas->color_id,
                .program = PROGRAM_TEXTURE,
                .format = VERTEX_FORMAT_TEXTURED,
                .mode = GL_TRIANGLE_FAN,
                .first = append_vertex_data(VERTEX_FORMAT_TEXTURED, vertices, 16) / 4,
                .count = 4,
            },
        });
    }

    // Upload vertex data to the GPU...
    for (int i = 0; i < NUM_VERTEX_FORMATS; i++) {
        gl2_vertex_buffer *buffer = &g_vertex_buffers[i];

        if (buffer->size == 0) {
            continue;
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);

        if (buffer->size > buffer->vbo_size) {
            glBufferData(GL_ARRAY_BUFFER, buffer->size * sizeof(float),
                         buffer->array, GL_STREAM_DRAW);
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                            buffer->size * sizeof(float),
                            buffer->array);
        }

        buffer->size = 0;
    }

    // Force VBO pointer update
    g_state.vertex_format = -1;

    // Just in case
    glFlush();

    // Execute all pending rendering commands...
    for (unsigned int i = 0; i < g_command_buffer.size; i++) {
        execute_command(&g_command_buffer.array[i]);
    }

    // Reset size of the command buffer to 0
    g_command_buffer.size = 0;

    // Restore transformation stack
    g_state.current_matrix = 0;
    libqu_mat4_identity(g_state.matrix[0]);
    update_model_view();

    // Restore surface
    append_command(&(gl2_command) {
        .type = COMMAND_RESET_SURFACE,
        .surface.id = g_state.canvas_id,
    });
}

static void gl2_notify_display_resize(int width, int height)
{
    append_command(&(gl2_command) {
        .type = COMMAND_RESIZE,
        .size = { width, height },
    });
}

static qu_vec2i gl2_conv_cursor(qu_vec2i position)
{
    if (!g_state.use_canvas) {
        return position;
    }

    float dar = g_state.display_aspect;
    float car = g_state.canvas_aspect;
    float dw = g_state.display_width;
    float dh = g_state.display_height;
    float cw = g_state.canvas_width;
    float ch = g_state.canvas_height;

    if (dar > car) {
        float x_scale = dh / ch;
        float x_offset = (dw - (cw * x_scale)) / (x_scale * 2.0f);

        return (qu_vec2i) {
            .x = (position.x * ch) / dh - x_offset,
            .y = (position.y / dh) * ch,
        };
    } else {
        float y_scale = dw / cw;
        float y_offset = (dh - (ch * y_scale)) / (y_scale * 2.0f);

        return (qu_vec2i) {
            .x = (position.x / dw) * cw,
            .y = (position.y * cw) / dw - y_offset,
        };
    }
}

static qu_vec2i gl2_conv_cursor_delta(qu_vec2i delta)
{
    if (!g_state.use_canvas) {
        return delta;
    }

    float dar = g_state.display_aspect;
    float car = g_state.canvas_aspect;
    float dw = g_state.display_width;
    float dh = g_state.display_height;
    float cw = g_state.canvas_width;
    float ch = g_state.canvas_height;

    if (g_state.display_aspect > g_state.canvas_aspect) {
        return (qu_vec2i) {
            .x = (delta.x * ch) / dh,
            .y = (delta.y / dh) * ch,
        };
    } else {
        return (qu_vec2i) {
            .x = (delta.x / dw) * cw,
            .y = (delta.y * cw) / dw,
        };
    }
}

//------------------------------------------------------------------------------

#endif // QU_GRAPHICS_GL2_IMPL_H
