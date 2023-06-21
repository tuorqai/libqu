//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#if !defined(QU_DISABLE_GLES2)

//------------------------------------------------------------------------------

#include <math.h>
#include <string.h>

#include "qu_array.h"
#include "qu_graphics.h"
#include "qu_image.h"
#include "qu_log.h"
#include "qu_math.h"

//------------------------------------------------------------------------------

#include <GLES2/gl2.h>

//------------------------------------------------------------------------------

#define COMMAND_NONE                    (0)
#define COMMAND_CLEAR                   (1)
#define COMMAND_DRAW                    (2)
#define COMMAND_SET_SURFACE             (3)
#define COMMAND_RESET_SURFACE           (4)
#define COMMAND_SET_VIEW                (5)
#define COMMAND_RESET_VIEW              (6)
#define COMMAND_PUSH_MATRIX             (7)
#define COMMAND_POP_MATRIX              (8)
#define COMMAND_TRANSLATE               (9)
#define COMMAND_SCALE                   (10)
#define COMMAND_ROTATE                  (11)
#define COMMAND_RESIZE                  (12)

#define VERTEX_ATTRIB_POSITION          (0)
#define VERTEX_ATTRIB_COLOR             (1)
#define VERTEX_ATTRIB_TEXCOORD          (2)
#define NUM_VERTEX_ATTRIBS              (3)

#define VERTEX_ATTRIB_POSITION_BIT      (1 << VERTEX_ATTRIB_POSITION)
#define VERTEX_ATTRIB_COLOR_BIT         (1 << VERTEX_ATTRIB_COLOR)
#define VERTEX_ATTRIB_TEXCOORD_BIT      (1 << VERTEX_ATTRIB_TEXCOORD)

#define VERTEX_FORMAT_SOLID             (0)
#define VERTEX_FORMAT_TEXTURED          (1)
#define NUM_VERTEX_FORMATS              (2)

#define SHADER_VERTEX                   (0)
#define SHADER_SOLID                    (1)
#define SHADER_TEXTURED                 (2)
#define SHADER_CANVAS                   (3)
#define NUM_SHADERS                     (4)

#define PROGRAM_SHAPE                   (0)
#define PROGRAM_TEXTURE                 (1)
#define PROGRAM_CANVAS                  (2)
#define NUM_PROGRAMS                    (3)

#define UNIFORM_PROJECTION              (0)
#define UNIFORM_MODELVIEW               (1)
#define UNIFORM_COLOR                   (2)
#define NUM_UNIFORMS                    (3)

#define UNIFORM_PROJECTION_BIT          (1 << UNIFORM_PROJECTION)
#define UNIFORM_MODELVIEW_BIT           (1 << UNIFORM_MODELVIEW)
#define UNIFORM_COLOR_BIT               (1 << UNIFORM_COLOR)

#define MAX_MATRICES                    (32)

//------------------------------------------------------------------------------

struct vertex_attrib_desc
{
    GLchar *name;
    int size;
};

struct vertex_format_desc
{
    int mask;
};

struct shader_desc
{
    GLenum type;
    GLchar *identifier;
    GLchar *source;
};

struct program_desc
{
    GLchar *identifier;
    int vs;
    int fs;
};

struct uniform_desc
{
    GLchar *name;
};

struct texture
{
    GLuint handle;
    GLuint width;
    GLuint height;

    GLint channels;
    GLenum format;
};

struct surface
{
    GLuint handle;
    GLuint depth;
    int32_t color_id;
    int width;
    int height;
};

struct state
{
    bool use_canvas;
    bool update_view_on_resize;

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

    float matrix[MAX_MATRICES][64];
    int current_matrix;
};

struct command
{
    int type;
    
    union {
        struct {
            qu_color color;
        } clear;

        struct {
            qu_color color;
            int32_t texture_id;
            int program;
            int format;
            int mode;
            int first;
            int count;
        } draw;

        struct {
            int32_t id;
        } surface;

        struct {
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
};

struct command_buffer
{
    struct command *array;
    unsigned int size;
    unsigned int capacity;
};

struct vertex_buffer
{
    float *array;
    unsigned int size;
    unsigned int capacity;

    GLuint vbo;
    GLuint vbo_size;
};

struct program
{
    GLuint handle;
    GLint uniform_locations[NUM_UNIFORMS];
    uint32_t dirty;
};

struct uniforms
{
    float projection[16];
    float *model_view; // points to impl.state.matrix[n]
    float color[4];
};

static struct
{
    bool initialized;
    libqu_array *textures;
    libqu_array *surfaces;
    struct state state;
    struct command_buffer command_buffer;
    struct vertex_buffer vertex_buffer[NUM_VERTEX_FORMATS];
    struct program programs[NUM_PROGRAMS];
    struct uniforms uniforms;
} impl;

//------------------------------------------------------------------------------

static struct vertex_attrib_desc vertex_attrib_map[NUM_VERTEX_ATTRIBS] = {
    [VERTEX_ATTRIB_POSITION] = {
        .name = "a_position",
        .size = 2,
    },
    [VERTEX_ATTRIB_COLOR] = {
        .name = "a_color",
        .size = 4,
    },
    [VERTEX_ATTRIB_TEXCOORD] = {
        .name = "a_texCoord",
        .size = 2,
    },
};

static struct vertex_format_desc vertex_format_map[NUM_VERTEX_FORMATS] = {
    [VERTEX_FORMAT_SOLID] = {
        .mask = VERTEX_ATTRIB_POSITION_BIT,
    },
    [VERTEX_FORMAT_TEXTURED] = {
        .mask = VERTEX_ATTRIB_POSITION_BIT | VERTEX_ATTRIB_TEXCOORD_BIT
    },
};

static struct shader_desc shader_map[NUM_SHADERS] = {
    [SHADER_VERTEX] = {
        .type = GL_VERTEX_SHADER,
        .identifier = "SHADER_VERTEX",
        .source =
            "attribute vec2 a_position;\n"
            "attribute vec4 a_color;\n"
            "attribute vec2 a_texCoord;\n"
            "varying vec4 v_color;\n"
            "varying vec2 v_texCoord;\n"
            "uniform mat4 u_projection;\n"
            "uniform mat4 u_modelView;\n"
            "void main()\n"
            "{\n"
            "    v_texCoord = a_texCoord;\n"
            "    v_color = a_color;\n"
            "    vec4 position = vec4(a_position, 0.0, 1.0);\n"
            "    gl_Position = u_projection * u_modelView * position;\n"
            "}\n",
    },
    [SHADER_SOLID] = {
        .type = GL_FRAGMENT_SHADER,
        .identifier = "SHADER_SOLID",
        .source =
            "precision mediump float;\n"
            "uniform vec4 u_color;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = u_color;\n"
            "}\n",
    },
    [SHADER_TEXTURED] = {
        .type = GL_FRAGMENT_SHADER,
        .identifier = "SHADER_TEXTURED",
        .source =
            "precision mediump float;\n"
            "varying vec2 v_texCoord;\n"
            "uniform sampler2D u_texture;\n"
            "uniform vec4 u_color;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = texture2D(u_texture, v_texCoord) * u_color;\n"
            "}\n",
    },
    [SHADER_CANVAS] = {
        .type = GL_VERTEX_SHADER,
        .identifier = "SHADER_CANVAS",
        .source =
            "attribute vec2 a_position;\n"
            "attribute vec2 a_texCoord;\n"
            "varying vec2 v_texCoord;\n"
            "uniform mat4 u_projection;\n"
            "void main()\n"
            "{\n"
            "    v_texCoord = a_texCoord;\n"
            "    vec4 position = vec4(a_position, 0.0, 1.0);\n"
            "    gl_Position = u_projection * position;\n"
            "}\n",
    },
};

static struct uniform_desc uniform_map[NUM_UNIFORMS] = {
    [UNIFORM_PROJECTION] = {
        .name = "u_projection",
    },
    [UNIFORM_MODELVIEW] = {
        .name = "u_modelView",
    },
    [UNIFORM_COLOR] = {
        .name = "u_color",
    },
};

static struct program_desc program_map[NUM_PROGRAMS] = {
    [PROGRAM_SHAPE] = {
        .identifier = "PROGRAM_SHAPE",
        .vs = SHADER_VERTEX,
        .fs = SHADER_SOLID,
    },
    [PROGRAM_TEXTURE] = {
        .identifier = "PROGRAM_TEXTURE",
        .vs = SHADER_VERTEX,
        .fs = SHADER_TEXTURED,
    },
    [PROGRAM_CANVAS] = {
        .identifier = "PROGRAM_CANVAS",
        .vs = SHADER_CANVAS,
        .fs = SHADER_TEXTURED,
    },
};

//------------------------------------------------------------------------------

static void texture_dtor(void *data);
static void surface_dtor(void *data);
static int32_t create_surface(int width, int height);

static void unpack_color(qu_color c, float *a)
{
    a[0] = ((c >> 16) & 255) / 255.f;
    a[1] = ((c >>  8) & 255) / 255.f;
    a[2] = ((c >>  0) & 255) / 255.f;
    a[3] = ((c >> 24) & 255) / 255.f;
}

static bool initialize_shader_programs(void)
{
    GLuint shaders[NUM_SHADERS];

    for (int i = 0; i < NUM_SHADERS; i++) {
        shaders[i] = glCreateShader(shader_map[i].type);
        glShaderSource(shaders[i], 1, (GLchar const * const *) &shader_map[i].source, NULL);
        glCompileShader(shaders[i]);

        GLint success;
        glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &success);

        if (!success) {
            char buffer[256];
            glGetShaderInfoLog(shaders[i], 256, NULL, buffer);
            
            libqu_error("Failed to compile GLSL shader %s. Reason:\n%s\n",
                shader_map[i].identifier, buffer);

            return false;
        }

        libqu_info("Shader %s is compiled successfully.\n", shader_map[i].identifier);
    }

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        impl.programs[i].handle = glCreateProgram();

        glAttachShader(impl.programs[i].handle, shaders[program_map[i].vs]);
        glAttachShader(impl.programs[i].handle, shaders[program_map[i].fs]);

        for (int j = 0; j < NUM_VERTEX_ATTRIBS; j++) {
            glBindAttribLocation(impl.programs[i].handle, j, vertex_attrib_map[j].name);
        }

        glLinkProgram(impl.programs[i].handle);

        GLint success;
        glGetProgramiv(impl.programs[i].handle, GL_LINK_STATUS, &success);

        if (!success) {
            char buffer[256];
            glGetProgramInfoLog(impl.programs[i].handle, 256, NULL, buffer);
            
            libqu_error("Failed to link GLSL program: %s\n", buffer);

            return false;
        }

        for (int j = 0; j < NUM_UNIFORMS; j++) {
            impl.programs[i].uniform_locations[j] =
                glGetUniformLocation(impl.programs[i].handle, uniform_map[j].name);
        }

        impl.programs[i].dirty = 0xffffffff;

        libqu_info("Shader program %s is compiled successfully.\n", program_map[i].identifier);
    }

    for (int i = 0; i < NUM_SHADERS; i++) {
        glDeleteShader(shaders[i]);
    }

    return true;
}

static bool initialize_vertex_buffers(void)
{
    for (int i = 0; i < NUM_VERTEX_FORMATS; i++) {
        glGenBuffers(1, &impl.vertex_buffer[i].vbo);
    }

    return true;
}

//------------------------------------------------------------------------------
// State management

static void update_canvas_coords(int w_display, int h_display)
{
    struct surface *canvas = libqu_array_get(impl.surfaces, impl.state.canvas_id);

    float ard = impl.state.display_aspect;
    float arc = impl.state.canvas_aspect;

    if (ard > arc) {
        impl.state.canvas_ax = (w_display / 2.f) - ((arc / ard) * w_display / 2.f);
        impl.state.canvas_ay = 0.f;
        impl.state.canvas_bx = (w_display / 2.f) + ((arc / ard) * w_display / 2.f);
        impl.state.canvas_by = h_display;
    } else {
        impl.state.canvas_ax = 0.f;
        impl.state.canvas_ay = (h_display / 2.f) - ((ard / arc) * h_display / 2.f);
        impl.state.canvas_bx = w_display;
        impl.state.canvas_by = (h_display / 2.f) + ((ard / arc) * h_display / 2.f);
    }
}

static void upload_uniform(int uniform, GLuint location)
{
    switch (uniform) {
    case UNIFORM_PROJECTION:
        glUniformMatrix4fv(location, 1, GL_FALSE, impl.uniforms.projection);
        break;
    case UNIFORM_MODELVIEW:
        glUniformMatrix4fv(location, 1, GL_FALSE, impl.uniforms.model_view);
        break;
    case UNIFORM_COLOR:
        glUniform4fv(location, 1, impl.uniforms.color);
        break;
    default:
        break;
    }
}

static void refresh_program(int program)
{
    if (impl.state.program == program && !impl.programs[program].dirty) {
        return;
    }

    glUseProgram(impl.programs[program].handle);

    if (impl.programs[program].dirty) {
        for (int i = 0; i < NUM_UNIFORMS; i++) {
            if (impl.programs[program].dirty & (1 << i)) {
                upload_uniform(i, impl.programs[program].uniform_locations[i]);
            }
        }
    }

    impl.programs[program].dirty = 0;
    impl.state.program = program;
}

static void refresh_vertex_format(int format)
{
    if (impl.state.vertex_format == format) {
        return;
    }

    int mask = vertex_format_map[format].mask;

    glBindBuffer(GL_ARRAY_BUFFER, impl.vertex_buffer[format].vbo);

    GLsizei stride = 0;

    for (int i = 0; i < NUM_VERTEX_ATTRIBS; i++) {
        if (mask & (1 << i)) {
            glEnableVertexAttribArray(i);
            stride += sizeof(GLfloat) * vertex_attrib_map[i].size;
        } else {
            glDisableVertexAttribArray(i);
        }
    }

    GLsizei offset = 0;

    for (int i = 0; i < NUM_VERTEX_ATTRIBS; i++) {
        if (mask & (1 << i)) {
            glVertexAttribPointer(i, vertex_attrib_map[i].size, GL_FLOAT,
                GL_FALSE, stride, (void *) offset);
            offset += sizeof(GLfloat) * vertex_attrib_map[i].size;
        }
    }

    impl.state.vertex_format = format;
}

static void refresh_clear_color(qu_color color)
{
    if (impl.state.clear_color == color) {
        return;
    }

    float r = ((color >> 16) & 255) / 255.f;
    float g = ((color >>  8) & 255) / 255.f;
    float b = ((color >>  0) & 255) / 255.f;
    float a = ((color >> 24) & 255) / 255.f;

    glClearColor(r, g, b, a);

    impl.state.clear_color = color;
}

static void update_draw_color_uniform(qu_color color)
{
    if (impl.state.draw_color == color) {
        return;
    }

    impl.uniforms.color[0] = ((color >> 16) & 255) / 255.f;
    impl.uniforms.color[1] = ((color >>  8) & 255) / 255.f;
    impl.uniforms.color[2] = ((color >>  0) & 255) / 255.f;
    impl.uniforms.color[3] = ((color >> 24) & 255) / 255.f;

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        impl.programs[i].dirty |= UNIFORM_COLOR_BIT;
    }

    impl.state.draw_color = color;
}

static void update_projection_uniform(float x, float y, float w, float h, float rotation)
{
    float l = x - (w / 2.f);
    float r = x + (w / 2.f);
    float b = y + (h / 2.f);
    float t = y - (h / 2.f);

    float *m = impl.uniforms.projection;

    libqu_mat4_ortho(m, l, r, b, t);

    if (rotation != 0.f) {
        libqu_mat4_translate(m, x, y, 0.f);
        libqu_mat4_rotate(m, QU_DEG2RAD(rotation), 0.f, 0.f, 1.f);
        libqu_mat4_translate(m, -x, -y, 0.f);
    }

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        impl.programs[i].dirty |= UNIFORM_PROJECTION_BIT;
    }
}

static void update_modelview_uniform(void)
{
    impl.uniforms.model_view = impl.state.matrix[impl.state.current_matrix];

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        impl.programs[i].dirty |= UNIFORM_MODELVIEW_BIT;
    }
}

static bool refresh_texture(int32_t id)
{
    if (impl.state.texture_id == id) {
        return true;
    }

    if (id == 0) {
        glBindTexture(GL_TEXTURE_2D, 0);
        impl.state.texture_id = 0;
        return true;
    }

    struct texture *texture = libqu_array_get(impl.textures, id);

    if (!texture) {
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture->handle);

    impl.state.texture_id = id;

    return true;
}

static bool refresh_surface(int32_t id)
{
    if (impl.state.surface_id == id) {
        return true;
    }

    GLuint handle;
    GLsizei width, height;

    if (id == 0) {
        handle = 0;
        width = impl.state.display_width;
        height = impl.state.display_height;
    } else {
        struct surface *surface = libqu_array_get(impl.surfaces, id);

        if (!surface) {
            return false;
        }

        handle = surface->handle;
        width = surface->width;
        height = surface->height;
    }

    // Restore view
    update_projection_uniform(width / 2.f, height / 2.f, width, height, 0.f);

    // Restore transformation stack
    impl.state.current_matrix = 0;
    libqu_mat4_identity(impl.state.matrix[0]);
    update_modelview_uniform();

    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glViewport(0, 0, width, height);
    update_projection_uniform(x_view, y_view, w_view, h_view, r_view);

    impl.state.surface_id = id;
    return true;
}

static void refresh_view(float x, float y, float w, float h, float rotation)
{
    update_projection_uniform(x, y, w, h, rotation);
}

static void refresh_view_to_default(void)
{
    GLsizei width, height;

    if (impl.state.surface_id == 0) {
        width = impl.state.display_width;
        height = impl.state.display_height;
    } else {
        struct surface *surface = libqu_array_get(impl.surfaces, impl.state.surface_id);

        if (!surface) {
            return;
        }

        width = surface->width;
        height = surface->height;
    }

    update_projection_uniform(width / 2.f, height / 2.f, width, height, 0.f);
}

//------------------------------------------------------------------------------
// Commands

static void exec_set_surface(int32_t id)
{
    refresh_surface(id);
}

static void exec_reset_surface(void)
{
    if (impl.state.use_canvas) {
        refresh_surface(impl.state.canvas_id);
    } else {
        refresh_surface(0);
    }
}

static void exec_push_matrix(void)
{
    if (impl.state.current_matrix == (MAX_MATRICES - 1)) {
        libqu_warning("Can't qu_push_matrix(): limit of %d matrices reached.\n", MAX_MATRICES);
        return;
    }

    float *next = impl.state.matrix[impl.state.current_matrix + 1];
    float *current = impl.state.matrix[impl.state.current_matrix];

    libqu_mat4_copy(next, current);

    impl.state.current_matrix++;
}

static void exec_pop_matrix(void)
{
    if (impl.state.current_matrix == 0) {
        libqu_warning("Can't qu_pop_matrix(): already at the first matrix.\n");
        return;
    }

    impl.state.current_matrix--;
}

static void exec_translate(float x, float y)
{
    float *matrix = impl.state.matrix[impl.state.current_matrix];
    libqu_mat4_translate(matrix, x, y, 0.f);
    update_modelview_uniform();
}

static void exec_scale(float x, float y)
{
    float *matrix = impl.state.matrix[impl.state.current_matrix];
    libqu_mat4_scale(matrix, x, y, 1.f);
    update_modelview_uniform();
}

static void exec_rotate(float degrees)
{
    float *matrix = impl.state.matrix[impl.state.current_matrix];
    libqu_mat4_rotate(matrix, QU_DEG2RAD(degrees), 0.f, 0.f, 1.f);
    update_modelview_uniform();
}

static void exec_resize(int width, int height)
{
    impl.state.display_width = width;
    impl.state.display_height = height;
    impl.state.display_aspect = width / (float) height;

    if (impl.state.use_canvas) {
        update_canvas_coords(width, height);
    }

    if (impl.state.surface_id == 0) {
        glViewport(0, 0, width, height);
        update_projection_uniform(width / 2.f, height / 2.f, width, height, 0.f);
    }
}

//------------------------------------------------------------------------------
// Command and vertex buffer management

static void append_command(struct command const *command)
{
    struct command_buffer *buffer = &impl.command_buffer;

    if (buffer->size == buffer->capacity) {
        unsigned int next_capacity = buffer->capacity * 2;

        if (!next_capacity) {
            next_capacity = 256;
        }

        struct command *next_array = realloc(buffer->array,
            sizeof(struct command) * next_capacity);

        if (!next_array) {
            return;
        }

        libqu_debug("GLES 2.0: grow command array [%d -> %d]\n",
            buffer->capacity, next_capacity);

        buffer->array = next_array;
        buffer->capacity = next_capacity;
    }

    memcpy(&buffer->array[buffer->size++], command, sizeof(struct command));
}

static void execute_command(struct command *command)
{
    switch (command->type) {
        case COMMAND_CLEAR:
            refresh_clear_color(command->clear.color);
            glClear(GL_COLOR_BUFFER_BIT);
            break;
        case COMMAND_DRAW:
            update_draw_color_uniform(command->draw.color);
            refresh_texture(command->draw.texture_id);
            refresh_program(command->draw.program);
            refresh_vertex_format(command->draw.format);
            glDrawArrays(command->draw.mode, command->draw.first, command->draw.count);
            break;
        case COMMAND_SET_SURFACE:
            exec_set_surface(command->surface.id);
            break;
        case COMMAND_RESET_SURFACE:
            exec_reset_surface();
            break;
        case COMMAND_SET_VIEW:
            refresh_view(command->view.x, command->view.y,
                command->view.w, command->view.h,
                command->view.r);
            break;
        case COMMAND_RESET_VIEW:
            refresh_view_to_default();
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

static int append_vertex_data(int format, float const *data, int size)
{
    struct vertex_buffer *buffer = &impl.vertex_buffer[format];

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

static void initialize(qu_params const *params)
{
    memset(&impl, 0, sizeof(impl));

    impl.textures = libqu_create_array(sizeof(struct texture), texture_dtor);
    impl.surfaces = libqu_create_array(sizeof(struct surface), surface_dtor);

    if (!initialize_shader_programs()) {
        return;
    }

    if (!initialize_vertex_buffers()) {
        return;
    }

    impl.state.use_canvas = params->enable_canvas;

    impl.state.display_width = params->display_width;
    impl.state.display_height = params->display_height;
    impl.state.display_aspect = params->display_width / (float) params->display_height;

    if (impl.state.use_canvas) {
        impl.state.canvas_width = params->canvas_width;
        impl.state.canvas_height = params->canvas_height;
        impl.state.canvas_aspect = params->canvas_width / (float) params->canvas_height;

        impl.state.canvas_id = create_surface(impl.state.canvas_width, impl.state.canvas_height);

        if (!impl.state.canvas_id) {
            libqu_error("Failed to initialize default framebuffer.\n");
            return;
        }

        update_canvas_coords(impl.state.display_width, impl.state.display_height);
    }

    append_command(&(struct command) {
        .type = COMMAND_RESET_SURFACE,
    });

    impl.state.texture_id = -1;
    impl.state.surface_id = -1;
    impl.state.program = -1;
    impl.state.vertex_format = -1;
    impl.state.clear_color = 0;
    impl.state.draw_color = 0;

    libqu_mat4_ortho(impl.uniforms.projection, 0.f,
        params->display_width, params->display_height, 0.f);

    impl.uniforms.model_view = impl.state.matrix[0];
    impl.state.current_matrix = 0;
    libqu_mat4_identity(impl.state.matrix[0]);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    libqu_info("OpenGL ES graphics module initialized.\n");
    libqu_info("OpenGL vendor: %s\n", glGetString(GL_VENDOR));
    libqu_info("OpenGL version: %s\n", glGetString(GL_VERSION));
    libqu_info("OpenGL renderer: %s\n", glGetString(GL_RENDERER));

    impl.initialized = true;
}

static void terminate(void)
{
    libqu_destroy_array(impl.surfaces);
    libqu_destroy_array(impl.textures);

    free(impl.command_buffer.array);

    for (int i = 0; i < NUM_PROGRAMS; i++) {
        glDeleteProgram(impl.programs[i].handle);
    }

    if (impl.initialized) {
        libqu_info("OpenGL graphics module terminated.\n");
        impl.initialized = false;
    }
}

static bool is_initialized(void)
{
    return impl.initialized;
}

static void swap(void)
{
    // If using canvas, then draw it in the default framebuffer
    if (impl.state.use_canvas) {
        append_command(&(struct command) {
            .type = COMMAND_SET_SURFACE,
            .surface.id = 0,
        });

        append_command(&(struct command) {
            .type = COMMAND_CLEAR,
            .clear.color = 0xff000000,
        });

        struct surface *canvas = libqu_array_get(impl.surfaces, impl.state.canvas_id);

        float vertices[] = {
            impl.state.canvas_ax, impl.state.canvas_ay, 0.f, 1.f,
            impl.state.canvas_bx, impl.state.canvas_ay, 1.f, 1.f,
            impl.state.canvas_bx, impl.state.canvas_by, 1.f, 0.f,
            impl.state.canvas_ax, impl.state.canvas_by, 0.f, 0.f,
        };

        append_command(&(struct command) {
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
        struct vertex_buffer *buffer = &impl.vertex_buffer[i];

        if (buffer->size == 0) {
            continue;
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);

        if (buffer->size > buffer->vbo_size) {
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer->size,
                buffer->array, GL_STREAM_DRAW);
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * buffer->size,
                buffer->array);
        }

        buffer->size = 0;
    }

    // Force VBO pointer update
    impl.state.vertex_format = -1;

    // Just in case
    glFlush();

    // Execute all pending rendering commands...
    for (unsigned int i = 0; i < impl.command_buffer.size; i++) {
        execute_command(&impl.command_buffer.array[i]);
    }

    // Reset size of the command buffer to 0
    impl.command_buffer.size = 0;

    // Restore transformation stack
    impl.state.current_matrix = 0;
    libqu_mat4_identity(impl.state.matrix[0]);
    update_modelview_uniform();

    // Restore surface
    append_command(&(struct command) {
        .type = COMMAND_RESET_SURFACE,
        .surface.id = impl.state.canvas_id,
    });
}

static void notify_display_resize(int width, int height)
{
    append_command(&(struct command) {
        .type = COMMAND_RESIZE,
        .size = {width, height},
    });
}

static qu_vec2i conv_cursor(qu_vec2i position)
{
    if (!impl.state.use_canvas) {
        return position;
    }

    float dar = impl.state.display_aspect;
    float car = impl.state.canvas_aspect;
    float dw = impl.state.display_width;
    float dh = impl.state.display_height;
    float cw = impl.state.canvas_width;
    float ch = impl.state.canvas_height;

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

static qu_vec2i conv_cursor_delta(qu_vec2i delta)
{
    if (!impl.state.use_canvas) {
        return delta;
    }

    float dar = impl.state.display_aspect;
    float car = impl.state.canvas_aspect;
    float dw = impl.state.display_width;
    float dh = impl.state.display_height;
    float cw = impl.state.canvas_width;
    float ch = impl.state.canvas_height;

    if (impl.state.display_aspect > impl.state.canvas_aspect) {
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
// Views

static void set_view(float x, float y, float w, float h, float rotation)
{
    append_command(&(struct command) {
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

static void reset_view(void)
{
    append_command(&(struct command) {
        .type = COMMAND_RESET_VIEW,
    });
}

//------------------------------------------------------------------------------
// Transformation

static void push_matrix(void)
{
    append_command(&(struct command) {
        .type = COMMAND_PUSH_MATRIX,
    });
}

static void pop_matrix(void)
{
    append_command(&(struct command) {
        .type = COMMAND_POP_MATRIX,
    });
}

static void translate(float x, float y)
{
    append_command(&(struct command) {
        .type = COMMAND_TRANSLATE,
        .view.x = x,
        .view.y = y,
    });
}

static void scale(float x, float y)
{
    append_command(&(struct command) {
        .type = COMMAND_SCALE,
        .view.x = x,
        .view.y = y,
    });
}

static void rotate(float degrees)
{
    append_command(&(struct command) {
        .type = COMMAND_ROTATE,
        .view.r = degrees,
    });
}

//------------------------------------------------------------------------------
// Primitives

static void make_circle(float x, float y, float radius, float *data, int num)
{
    float angle = QU_DEG2RAD(360.f / num);
    
    for (int i = 0; i < num; i++) {
        data[2 * i + 0] = x + (radius * cosf(i * angle));
        data[2 * i + 1] = y + (radius * sinf(i * angle));
    }
}

static void clear(qu_color color)
{
    append_command(&(struct command) {
        .type = COMMAND_CLEAR,
        .clear = {
            .color = color,
        },
    });
}

static void draw_point(float x, float y, qu_color color)
{
    float vertices[] = {
        x, y,
    };

    append_command(&(struct command) {
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

static void draw_line(float ax, float ay, float bx, float by, qu_color color)
{
    float vertices[] = {
        ax, ay,
        bx, by,
    };

    append_command(&(struct command) {
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

static void draw_triangle(float ax, float ay, float bx, float by, float cx, float cy, qu_color outline, qu_color fill)
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
        append_command(&(struct command) {
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
        append_command(&(struct command) {
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

static void draw_rectangle(float x, float y, float w, float h, qu_color outline, qu_color fill)
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
        append_command(&(struct command) {
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
        append_command(&(struct command) {
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

static void draw_circle(float x, float y, float radius, qu_color outline, qu_color fill)
{
    int fill_alpha = (fill >> 24) & 255;
    int outline_alpha = (outline >> 24) & 255;

    float vertices[64];
    make_circle(x, y, radius, vertices, 32);

    int first = append_vertex_data(VERTEX_FORMAT_SOLID, vertices, 64) / 2;

    if (fill_alpha > 0) {
        append_command(&(struct command) {
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
        append_command(&(struct command) {
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
    struct texture *texture = data;
    glDeleteTextures(1, &texture->handle);
    // libqu_info("Deleted texture 0x%08x.\n", texture->id);
}

static int32_t create_texture(int width, int height, int channels)
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

    struct texture texture = { 0 };

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

    impl.state.texture_id = libqu_array_add(impl.textures, &texture);

    if (impl.state.texture_id > 0) {
        libqu_info("Created texture 0x%08x.\n", impl.state.texture_id);
    }

    return impl.state.texture_id;
}

static void update_texture(int32_t texture_id, int x, int y, int w, int h, uint8_t const *pixels)
{
    struct texture *texture = libqu_array_get(impl.textures, texture_id);

    if (!texture || !refresh_texture(texture_id)) {
        return;
    }

    if (x == 0 && y == 0 && w == -1 && h == -1) {
        glTexImage2D(GL_TEXTURE_2D, 0, texture->format,
            texture->width, texture->height, 0,
            texture->format, GL_UNSIGNED_BYTE, pixels);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
            texture->format, GL_UNSIGNED_BYTE, pixels);
    }
}

static int32_t load_texture(libqu_file *file)
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

    struct texture texture = { 0 };

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

    impl.state.texture_id = libqu_array_add(impl.textures, &texture);

    if (impl.state.texture_id > 0) {
        libqu_info("Loaded texture 0x%08x.\n", impl.state.texture_id);
    }

    libqu_delete_image(image);

    return impl.state.texture_id;
}

static void delete_texture(int32_t texture_id)
{
    struct texture *texture = libqu_array_get(impl.textures, texture_id);

    if (!texture) {
        return;
    }

    libqu_array_remove(impl.textures, texture_id);
}

static void set_texture_smooth(int32_t texture_id, bool smooth)
{
    if (!refresh_texture(texture_id)) {
        return;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
}

static void draw_texture(int32_t texture_id, float x, float y, float w, float h)
{
    float vertices[] = {
        x,      y,      0.f,    0.f,
        x + w,  y,      1.f,    0.f,
        x + w,  y + h,  1.f,    1.f,
        x,      y + h,  0.f,    1.f,
    };

    append_command(&(struct command) {
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

static void draw_subtexture(int32_t texture_id, float x, float y, float w, float h, float rx, float ry, float rw, float rh)
{
    struct texture *texture = libqu_array_get(impl.textures, texture_id);

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

    append_command(&(struct command) {
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

static void draw_text(int32_t texture_id, qu_color color, float const *data, int count)
{
    append_command(&(struct command) {
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
// Surfaces (framebuffers)

static void surface_dtor(void *data)
{
    struct surface *surface = data;

    libqu_array_remove(impl.textures, surface->color_id);

    glDeleteFramebuffers(1, &surface->handle);
    glDeleteRenderbuffers(1, &surface->depth);
}

static int32_t create_surface(int width, int height)
{
    if (width < 0 || height < 0) {
        return 0;
    }

    struct surface surface = { 0 };

    glGenFramebuffers(1, &surface.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, surface.handle);

    glGenRenderbuffers(1, &surface.depth);
    glBindRenderbuffer(GL_RENDERBUFFER, surface.depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, surface.depth);
    
    surface.color_id = create_texture(width, height, 4);

    if (surface.color_id == 0) {
        glDeleteFramebuffers(1, &surface.handle);
        return 0;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    struct texture *texture = libqu_array_get(impl.textures, surface.color_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->handle, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        libqu_error("Failed to create OpenGL framebuffer.\n");
        return 0;
    }

    surface.width = width;
    surface.height = height;

    impl.state.surface_id = libqu_array_add(impl.surfaces, &surface);

    if (impl.state.surface_id == 0) {
        libqu_error("Failed to create surface: insufficient memory.\n");
        return 0;
    }

    return impl.state.surface_id;
}

static void delete_surface(int32_t id)
{
    libqu_array_remove(impl.surfaces, id);
}

static void set_surface(int32_t id)
{
    append_command(&(struct command) {
        .type = COMMAND_SET_SURFACE,
        .surface.id = id,
    });
}

static void reset_surface(void)
{
    append_command(&(struct command) {
        .type = COMMAND_RESET_SURFACE,
    });
}

static void draw_surface(int32_t id, float x, float y, float w, float h)
{
    struct surface *surface = libqu_array_get(impl.surfaces, id);

    if (!surface) {
        return;
    }

    float vertices[] = {
        x,      y,      0.f,    1.f,
        x + w,  y,      1.f,    1.f,
        x + w,  y + h,  1.f,    0.f,
        x,      y + h,  0.f,    0.f,
    };

    append_command(&(struct command) {
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

void libqu_construct_gles2_graphics(libqu_graphics *graphics)
{
    *graphics = (libqu_graphics) {
        .initialize = initialize,
        .terminate = terminate,
        .is_initialized = is_initialized,
        .swap = swap,
        .notify_display_resize = notify_display_resize,
        .conv_cursor = conv_cursor,
        .conv_cursor_delta = conv_cursor_delta,
        .set_view = set_view,
        .reset_view = reset_view,
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
        .create_surface = create_surface,
        .delete_surface = delete_surface,
        .set_surface = set_surface,
        .reset_surface = reset_surface,
        .draw_surface = draw_surface,
    };
}

//------------------------------------------------------------------------------

#endif // !defined(QU_DISABLE_GLES2)
