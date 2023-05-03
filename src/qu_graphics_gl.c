//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <math.h>
#include <string.h>
#include <GL/gl.h>

#include "qu_array.h"
#include "qu_gateway.h"
#include "qu_graphics.h"
#include "qu_halt.h"
#include "qu_image.h"
#include "qu_log.h"
#include "qu_util.h"

//------------------------------------------------------------------------------

#define STATE_USE_CANVAS                (0x01)
#define STATE_UPDATE_VIEW_ON_RESIZE     (0x02)

#define ATTRIB_POSITION                 (0x01)
#define ATTRIB_COLOR                    (0x02)
#define ATTRIB_TEXCOORD                 (0x04)

#define COLOR_BLACK                     (0xFF000000)
#define COLOR_WHITE                     (0xFFFFFFFF)

//------------------------------------------------------------------------------

struct view
{
    float x;
    float y;
    float w;
    float h;
    float r;
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
    GLuint width;
    GLuint height;
    int32_t texture_id;
    struct view view;
};

static struct
{
    bool initialized;
    uint32_t state;

    libqu_array *textures;
    libqu_array *surfaces;

    int display_width;
    int display_height;
    float display_aspect;
    struct view display_view;

    qu_color clear_color;
    qu_color draw_color;
    int attrib_mask;
    int texture_id;
    float texture_width;
    float texture_height;
    int surface_id;

    struct surface *canvas;
    int32_t canvas_id;
    float canvas_aspect;
    float canvas_ax;
    float canvas_ay;
    float canvas_bx;
    float canvas_by;
} impl;

//------------------------------------------------------------------------------
// Projection

static struct view make_default_view(int width, int height)
{
    return (struct view) {
        .x = width / 2.f,
        .y = height / 2.f,
        .w = width,
        .h = height,
        .r = 0.f,
    };
}

static void apply_view(struct view view)
{
    float l = view.x - (view.w / 2.f);
    float r = view.x + (view.w / 2.f);
    float b = view.y + (view.h / 2.f);
    float t = view.y - (view.h / 2.f);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    glOrtho(l, r, b, t, -1.0, +1.0);

    if (view.r != 0.f) {
        glTranslatef(view.x, view.y, 0.f);
        glRotatef(view.r, 0.f, 0.f, 1.f);
        glTranslatef(-view.x, -view.y, 0.f);
    }

    glMatrixMode(GL_MODELVIEW);
}

//------------------------------------------------------------------------------
// Textures

static void texture_dtor(void *data)
{
    struct texture *texture = data;
    glDeleteTextures(1, &texture->handle);
}

static bool initialize_textures(void)
{
    impl.textures = libqu_create_array(sizeof(struct texture), texture_dtor);

    if (!impl.textures) {
        return false;
    }

    impl.texture_id = 0;
    impl.texture_width = 1.f;
    impl.texture_height = 1.f;

    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    return true;
}

static void terminate_textures(void)
{
    glDisable(GL_TEXTURE_2D);
    libqu_destroy_array(impl.textures);
}

static bool apply_texture(int32_t id)
{
    if (impl.texture_id == id) {
        return true;
    }

    if (id == 0) {
        glBindTexture(GL_TEXTURE_2D, 0);

        impl.texture_id = 0;
        impl.texture_width = 1.f;
        impl.texture_height = 1.f;

        return true;
    }

    struct texture *texture = libqu_array_get(impl.textures, id);

    if (!texture) {
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture->handle);

    impl.texture_id = id;
    impl.texture_width = (float) texture->width;
    impl.texture_height = (float) texture->height;

    return true;
}

static int32_t generate_texture(int width, int height, int channels, uint8_t const *pixels)
{
    if ((width < 0) || (height < 0)) {
        return 0;
    }

    if ((channels < 1) || (channels > 4)) {
        return 0;
    }

    GLenum format;

    switch (channels) {
    case 1:
        format = GL_LUMINANCE;
        break;
    case 2:
        format = GL_LUMINANCE_ALPHA;
        break;
    case 3:
        format = GL_RGB;
        break;
    default:
        format = GL_RGBA;
        break;
    }

    GLuint handle;

    glGenTextures(1, &handle);

    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height,
        0, format, GL_UNSIGNED_BYTE, pixels);

    impl.texture_id = libqu_array_add(impl.textures, &(struct texture) {
        .handle = handle,
        .width = width,
        .height = height,
        .channels = channels,
        .format = format,
    });

    if (impl.texture_id > 0) {
        impl.texture_width = (float) width;
        impl.texture_height = (float) height;
    }

    return impl.texture_id;
}

static int32_t create_texture(int width, int height, int channels)
{
    int32_t id = generate_texture(width, height, channels, NULL);

    if (!id) {
        return 0;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return id;
}

static int32_t load_texture(libqu_file *file)
{
    libqu_image *image = libqu_load_image(file);

    libqu_fclose(file);

    if (!image) {
        return 0;
    }

    int32_t id = generate_texture(image->width, image->height, image->channels, image->pixels);

    if (!id) {
        return 0;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return id;
}

static void update_texture(int32_t id, int x, int y, int w, int h, uint8_t const *pixels)
{
    if (!apply_texture(id)) {
        return;
    }

    struct texture *texture = libqu_array_get(impl.textures, id);

    if (x == 0 && y == 0 && w == -1 && h == -1) {
        glTexImage2D(GL_TEXTURE_2D, 0, texture->channels,
            texture->width, texture->height, 0,
            texture->format, GL_UNSIGNED_BYTE, pixels);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, texture->format, GL_UNSIGNED_BYTE, pixels);
    }
}

static void delete_texture(int32_t id)
{
    libqu_array_remove(impl.textures, id);
}

static void set_texture_smooth(int32_t id, bool smooth)
{
    if (!apply_texture(id)) {
        return;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
}

//------------------------------------------------------------------------------
// Framebuffers

static PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT;
static PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
static PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
static PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
static PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
static PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
static PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT;
static PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
static PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
static PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
static PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
static PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
static PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
static PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
static PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;

static void surface_dtor(void *data)
{
    struct surface *surface = data;

    libqu_array_remove(impl.textures, surface->texture_id);
    glDeleteFramebuffersEXT(1, &surface->handle);
}

static bool initialize_framebuffers(void)
{
    impl.surfaces = libqu_create_array(sizeof(struct surface), surface_dtor);

    if (!impl.surfaces) {
        return false;
    }

    impl.surface_id = 0;

    char *extensions = libqu_strdup((char const *) glGetString(GL_EXTENSIONS));
    char *token = strtok(extensions, " ");
    bool found = false;

    while (token) {
        if (strcmp(token, "GL_EXT_framebuffer_object") == 0) {
            found = true;
            break;
        }

        token = strtok(NULL, " ");
    }

    free(extensions);

    if (!found) {
        return false;
    }

    glIsRenderbufferEXT = libqu_gl_proc_address("glIsRenderbufferEXT");
    glBindRenderbufferEXT = libqu_gl_proc_address("glBindRenderbufferEXT");
    glDeleteRenderbuffersEXT = libqu_gl_proc_address("glDeleteRenderbuffersEXT");
    glGenRenderbuffersEXT = libqu_gl_proc_address("glGenRenderbuffersEXT");
    glRenderbufferStorageEXT = libqu_gl_proc_address("glRenderbufferStorageEXT");
    glGetRenderbufferParameterivEXT = libqu_gl_proc_address("glGetRenderbufferParameterivEXT");
    glIsFramebufferEXT = libqu_gl_proc_address("glIsFramebufferEXT");
    glBindFramebufferEXT = libqu_gl_proc_address("glBindFramebufferEXT");
    glDeleteFramebuffersEXT = libqu_gl_proc_address("glDeleteFramebuffersEXT");
    glGenFramebuffersEXT = libqu_gl_proc_address("glGenFramebuffersEXT");
    glCheckFramebufferStatusEXT = libqu_gl_proc_address("glCheckFramebufferStatusEXT");
    glFramebufferTexture1DEXT = libqu_gl_proc_address("glFramebufferTexture1DEXT");
    glFramebufferTexture2DEXT = libqu_gl_proc_address("glFramebufferTexture2DEXT");
    glFramebufferTexture3DEXT = libqu_gl_proc_address("glFramebufferTexture3DEXT");
    glFramebufferRenderbufferEXT = libqu_gl_proc_address("glFramebufferRenderbufferEXT");
    glGetFramebufferAttachmentParameterivEXT = libqu_gl_proc_address("glGetFramebufferAttachmentParameterivEXT");
    glGenerateMipmapEXT = libqu_gl_proc_address("glGenerateMipmapEXT");

    return true;
}

static void terminate_framebuffers(void)
{
    libqu_destroy_array(impl.surfaces);
}

static void apply_framebuffer(int32_t id)
{
    if (impl.surface_id == id) {
        return;
    }

    if (id == 0) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glViewport(0, 0, impl.display_width, impl.display_height);
        apply_view(impl.display_view);

        impl.surface_id = 0;
        return;
    }

    struct surface *surface = libqu_array_get(impl.surfaces, id);

    if (!surface) {
        return;
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface->handle);
    glViewport(0, 0, surface->width, surface->height);
    apply_view(surface->view);

    impl.surface_id = id;
}

static int32_t generate_framebuffer(int width, int height)
{
    int32_t texture_id = generate_texture(width, height, 4, NULL);
    struct texture *texture = libqu_array_get(impl.textures, texture_id);

    if (!texture) {
        return 0;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint handle;

    glGenFramebuffersEXT(1, &handle);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, handle);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
        GL_TEXTURE_2D, texture->handle, 0);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        libqu_error("Failed to create OpenGL framebuffer.\n");
        return 0;
    }

    impl.surface_id = libqu_array_add(impl.surfaces, &(struct surface) {
        .handle = handle,
        .texture_id = texture_id,
        .width = width,
        .height = height,
        .view = make_default_view(width, height),
    });

    if (impl.surface_id == 0) {
        libqu_error("Failed to create surface: insufficient memory.\n");
        return 0;
    }

    return impl.surface_id;
}

static void delete_framebuffer(int32_t id)
{
    libqu_array_remove(impl.surfaces, id);
}

//------------------------------------------------------------------------------
// Drawing

static void apply_clear_color(qu_color color)
{
    if (impl.clear_color == color) {
        return;
    }

    float r = ((color >> 16) & 255) / 255.f;
    float g = ((color >>  8) & 255) / 255.f;
    float b = ((color >>  0) & 255) / 255.f;
    float a = ((color >> 24) & 255) / 255.f;

    glClearColor(r, g, b, a);

    impl.clear_color = color;
}

static void apply_draw_color(qu_color color)
{
    if (impl.draw_color == color) {
        return;
    }

    unsigned char r = (color >> 16) & 255;
    unsigned char g = (color >>  8) & 255;
    unsigned char b = (color >>  0) & 255;
    unsigned char a = (color >> 24) & 255;

    glColor4ub(r, g, b, a);

    impl.draw_color = color;
}

static void apply_attrib_mask(int mask)
{
    if (impl.attrib_mask == mask) {
        return;
    }

    if (mask & ATTRIB_POSITION) {
        glEnableClientState(GL_VERTEX_ARRAY);
    } else {
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    if (mask & ATTRIB_COLOR) {
        glEnableClientState(GL_COLOR_ARRAY);
    } else {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    if (mask & ATTRIB_TEXCOORD) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    } else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    impl.attrib_mask = mask;
}

static void clear(qu_color color)
{
    apply_clear_color(color);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void draw_primitive(qu_color color, float const *data, int count, GLenum mode)
{
    apply_texture(0);
    apply_draw_color(color);
    apply_attrib_mask(ATTRIB_POSITION);

    glVertexPointer(2, GL_FLOAT, 0, data);
    glDrawArrays(mode, 0, count);
}

static void draw_texture(int32_t id, qu_color color, float const *data, int count, GLenum mode)
{
    apply_texture(id);
    apply_draw_color(color);
    apply_attrib_mask(ATTRIB_POSITION | ATTRIB_TEXCOORD);

    glVertexPointer(2, GL_FLOAT, sizeof(float) * 4, data);
    glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 4, data + 2);
    glDrawArrays(mode, 0, count);
}

//------------------------------------------------------------------------------
// Canvas

static void update_canvas(int w_display, int h_display)
{
    float ard = impl.display_aspect;
    float arc = impl.canvas_aspect;

    if (ard > arc) {
        impl.canvas_ax = (w_display / 2.f) - ((arc / ard) * w_display / 2.f);
        impl.canvas_ay = 0.f;
        impl.canvas_bx = (w_display / 2.f) + ((arc / ard) * w_display / 2.f);
        impl.canvas_by = h_display;
    } else {
        impl.canvas_ax = 0.f;
        impl.canvas_ay = (h_display / 2.f) - ((ard / arc) * h_display / 2.f);
        impl.canvas_bx = w_display;
        impl.canvas_by = (h_display / 2.f) + ((ard / arc) * h_display / 2.f);
    }
}

static bool initialize_canvas(void)
{
    impl.canvas_id = generate_framebuffer(impl.display_width, impl.display_height);

    if (impl.canvas_id == 0) {
        return false;
    }

    impl.canvas_aspect = impl.display_aspect;
    impl.canvas = libqu_array_get(impl.surfaces, impl.canvas_id);

    update_canvas(impl.display_width, impl.display_height);
    apply_framebuffer(impl.canvas_id);

    return true;
}

static void draw_canvas(void)
{
    apply_framebuffer(0);
    clear(COLOR_BLACK);

    float vertices[] = {
        impl.canvas_ax, impl.canvas_ay, 0.f, 1.f,
        impl.canvas_bx, impl.canvas_ay, 1.f, 1.f,
        impl.canvas_bx, impl.canvas_by, 1.f, 0.f,
        impl.canvas_ax, impl.canvas_by, 0.f, 0.f,
    };

    draw_texture(impl.canvas->texture_id, COLOR_WHITE, vertices, 4, GL_TRIANGLE_FAN);
}

static qu_vec2i pixel_pos_to_canvas_pos(qu_vec2i position)
{
    float dw = impl.display_width;
    float dh = impl.display_height;
    float cw = impl.canvas->width;
    float ch = impl.canvas->height;

    if (impl.display_aspect > impl.canvas_aspect) {
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

static qu_vec2i pixel_delta_to_canvas_delta(qu_vec2i position)
{
    float dw = impl.display_width;
    float dh = impl.display_height;
    float cw = impl.canvas->width;
    float ch = impl.canvas->height;

    if (impl.display_aspect > impl.canvas_aspect) {
        return (qu_vec2i) {
            .x = (position.x * ch) / dh,
            .y = (position.y / dh) * ch,
        };
    } else {
        return (qu_vec2i) {
            .x = (position.x / dw) * cw,
            .y = (position.y * cw) / dw,
        };
    }
}

//------------------------------------------------------------------------------

void libqu_gl_initialize(qu_params const *params)
{
    memset(&impl, 0, sizeof(impl));

    impl.display_width = params->display_width;
    impl.display_height = params->display_height;
    impl.display_aspect = impl.display_width / (float) impl.display_height;
    impl.display_view = make_default_view(impl.display_width, impl.display_height);
    apply_view(impl.display_view);

    switch (params->screen_mode) {
    case QU_SCREEN_MODE_UPDATE_VIEW:
        impl.state |= STATE_UPDATE_VIEW_ON_RESIZE;
        break;
    case QU_SCREEN_MODE_USE_CANVAS:
        impl.state |= STATE_USE_CANVAS | STATE_UPDATE_VIEW_ON_RESIZE;
        break;
    default:
        break;
    }

    if (!initialize_textures()) {
        return;
    }

    if (!initialize_framebuffers()) {
        return;
    }

    if (impl.state & STATE_USE_CANVAS) {
        if (!initialize_canvas()) {
            return;
        }
    }

    impl.clear_color = COLOR_BLACK;
    impl.draw_color = COLOR_WHITE;

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glColor4f(1.f, 1.f, 1.f, 1.f);

    glDisable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    libqu_info("Legacy OpenGL graphics module initialized.\n");
    impl.initialized = true;
}

void libqu_gl_terminate(void)
{
    terminate_framebuffers();
    terminate_textures();

    if (impl.initialized) {
        libqu_info("Legacy OpenGL graphics module terminated.\n");
        impl.initialized = false;
    }
}

bool libqu_gl_is_initialized(void)
{
    return impl.initialized;
}

void libqu_gl_swap(void)
{
    if (impl.state & STATE_USE_CANVAS) {
        draw_canvas();
    }

    glFlush();

#ifndef NDEBUG
    GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        libqu_error("Following OpenGL error(s) occurred:\n");

        while (error != GL_NO_ERROR) {
            libqu_error("* error 0x%04x\n");
            error = glGetError();
        }

        libqu_halt("OpenGL is in an inconsistent state.\n");
    }
#endif

    if (impl.state & STATE_USE_CANVAS) {
        apply_framebuffer(impl.canvas_id);
    } else {
        apply_framebuffer(0);
    }
}

void libqu_gl_notify_display_resize(int width, int height)
{
    impl.display_width = width;
    impl.display_height = height;
    impl.display_aspect = width / (float) height;

    if (impl.state & STATE_USE_CANVAS) {
        update_canvas(width, height);
    }

    if (impl.state & STATE_UPDATE_VIEW_ON_RESIZE) {
        impl.display_view = make_default_view(impl.display_width, impl.display_height);
    }

    if (impl.surface_id == 0) {
        apply_view(impl.display_view);
        glViewport(0, 0, width, height);
    }
}

//------------------------------------------------------------------------------

qu_vec2i libqu_gl_conv_abs_position(qu_vec2i position)
{
    if (impl.state & STATE_USE_CANVAS) {
        return pixel_pos_to_canvas_pos(position);
    }

    return position;
}

qu_vec2i libqu_gl_conv_rel_position(qu_vec2i delta)
{
    if (impl.state & STATE_USE_CANVAS) {
        return pixel_delta_to_canvas_delta(delta);
    }

    return delta;
}

//------------------------------------------------------------------------------

void libqu_gl_set_view(float x, float y, float w, float h, float rotation)
{
    struct view view = {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .r = rotation
    };

    if (impl.surface_id == 0) {
        impl.display_view = view;
    } else {
        struct surface *surface = libqu_array_get(impl.surfaces, impl.surface_id);

        if (surface) {
            surface->view = view;
        }
    }

    apply_view(view);
}

void libqu_gl_reset_view(void)
{
    if (impl.surface_id == 0) {
        impl.display_view = make_default_view(impl.display_width, impl.display_height);
        apply_view(impl.display_view);
    } else {
        struct surface *surface = libqu_array_get(impl.surfaces, impl.surface_id);

        if (surface) {
            surface->view = make_default_view(surface->width, surface->height);
            apply_view(surface->view);
        }
    }
}

//------------------------------------------------------------------------------

void libqu_gl_clear(qu_color color)
{
    clear(color);
}

void libqu_gl_draw_point(float x, float y, qu_color color)
{
    GLfloat vertices[] = {
        x, y,
    };

    draw_primitive(color, vertices, 1, GL_POINTS);
}

void libqu_gl_draw_line(float ax, float ay, float bx, float by, qu_color color)
{
    GLfloat vertices[] = {
        ax, ay,
        bx, by,
    };

    draw_primitive(color, vertices, 2, GL_LINES);
}

void libqu_gl_draw_triangle(float ax, float ay, float bx, float by, float cx, float cy, qu_color outline, qu_color fill)
{
    float vertices[] = {
        ax, ay,
        bx, by,
        cx, cy,
    };

    int outline_alpha = (outline >> 24) & 255;
    int fill_alpha = (fill >> 24) & 255;

    if (fill_alpha > 0) {
        draw_primitive(fill, vertices, 3, GL_TRIANGLES);
    }

    if (outline_alpha > 0) {
        draw_primitive(outline, vertices, 3, GL_LINE_LOOP);
    }
}

void libqu_gl_draw_rectangle(float x, float y, float w, float h, qu_color outline, qu_color fill)
{
    float vertices[] = {
        x,      y,
        x + w,  y,
        x + w,  y + h,
        x,      y + h,
    };

    int outline_alpha = (outline >> 24) & 255;
    int fill_alpha = (fill >> 24) & 255;

    if (fill_alpha > 0) {
        draw_primitive(fill, vertices, 4, GL_TRIANGLE_FAN);
    }

    if (outline_alpha > 0) {
        draw_primitive(outline, vertices, 4, GL_LINE_LOOP);
    }
}

void libqu_gl_draw_circle(float x, float y, float radius, qu_color outline, qu_color fill)
{
    float vertices[64];
    libqu_make_circle(x, y, radius, vertices, 32);

    int outline_alpha = (outline >> 24) & 255;
    int fill_alpha = (fill >> 24) & 255;

    if (fill_alpha > 0) {
        draw_primitive(fill, vertices, 32, GL_TRIANGLE_FAN);
    }

    if (outline_alpha > 0) {
        draw_primitive(outline, vertices, 32, GL_LINE_LOOP);
    }
}

//------------------------------------------------------------------------------

int32_t libqu_gl_create_texture(int width, int height, int channels)
{
    return create_texture(width, height, channels);
}

void libqu_gl_update_texture(int32_t id, int x, int y, int w, int h, uint8_t const *pixels)
{
    update_texture(id, x, y, w, h, pixels);
}

int32_t libqu_gl_load_texture(libqu_file *file)
{
    return load_texture(file);
}

void libqu_gl_delete_texture(int32_t id)
{
    delete_texture(id);
}

void libqu_gl_set_texture_smooth(int32_t id, bool smooth)
{
    set_texture_smooth(id, smooth);
}

void libqu_gl_draw_texture(int32_t id, float x, float y, float w, float h)
{
    float vertices[] = {
        x,      y,      0.f,    0.f,
        x + w,  y,      1.f,    0.f,
        x + w,  y + h,  1.f,    1.f,
        x,      y + h,  0.f,    1.f,
    };

    draw_texture(id, COLOR_WHITE, vertices, 4, GL_TRIANGLE_FAN);
}

void libqu_gl_draw_subtexture(int32_t id, float x, float y, float w, float h, float rx, float ry, float rw, float rh)
{
    float s = rx / impl.texture_width;
    float t = ry / impl.texture_height;
    float u = s + rw / impl.texture_width;
    float v = t + rh / impl.texture_height;

    float vertices[] = {
        x,      y,      s,  t,
        x + w,  y,      u,  t,
        x + w,  y + h,  u,  v,
        x,      y + h,  s,  v,
    };

    draw_texture(id, COLOR_WHITE, vertices, 4, GL_TRIANGLE_FAN);
}

void libqu_gl_draw_text(int32_t id, qu_color color, float const *data, int count)
{
    draw_texture(id, color, data, count, GL_TRIANGLES);
}

//------------------------------------------------------------------------------

int32_t libqu_gl_create_surface(int width, int height)
{
    return generate_framebuffer(width, height);
}

void libqu_gl_delete_surface(int32_t id)
{
    delete_framebuffer(id);
}

void libqu_gl_set_surface(int32_t id)
{
    apply_framebuffer(id);
}

void libqu_gl_reset_surface(void)
{
    if (impl.state & STATE_USE_CANVAS) {
        apply_framebuffer(impl.canvas_id);
    } else {
        apply_framebuffer(0);
    }
}

void libqu_gl_draw_surface(int32_t id, float x, float y, float w, float h)
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

    draw_texture(surface->texture_id, COLOR_WHITE, vertices, 4, GL_TRIANGLE_FAN);
}

//------------------------------------------------------------------------------

void libqu_construct_gl_graphics(libqu_graphics *graphics)
{
    *graphics = (libqu_graphics) {
        .initialize = libqu_gl_initialize,
        .terminate = libqu_gl_terminate,
        .is_initialized = libqu_gl_is_initialized,
        .swap = libqu_gl_swap,
        .notify_display_resize = libqu_gl_notify_display_resize,
        .conv_cursor = libqu_gl_conv_abs_position,
        .conv_cursor_delta = libqu_gl_conv_rel_position,
        .set_view = libqu_gl_set_view,
        .reset_view = libqu_gl_reset_view,
        .clear = libqu_gl_clear,
        .draw_point = libqu_gl_draw_point,
        .draw_line = libqu_gl_draw_line,
        .draw_triangle = libqu_gl_draw_triangle,
        .draw_rectangle = libqu_gl_draw_rectangle,
        .draw_circle = libqu_gl_draw_circle,
        .create_texture = libqu_gl_create_texture,
        .update_texture = libqu_gl_update_texture,
        .load_texture = libqu_gl_load_texture,
        .delete_texture = libqu_gl_delete_texture,
        .set_texture_smooth = libqu_gl_set_texture_smooth,
        .draw_texture = libqu_gl_draw_texture,
        .draw_subtexture = libqu_gl_draw_subtexture,
        .draw_text = libqu_gl_draw_text,
        .create_surface = libqu_gl_create_surface,
        .delete_surface = libqu_gl_delete_surface,
        .set_surface = libqu_gl_set_surface,
        .reset_surface = libqu_gl_reset_surface,
        .draw_surface = libqu_gl_draw_surface,
    };
}

//------------------------------------------------------------------------------
