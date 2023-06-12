
#ifndef QU_GLEXT_H
#define QU_GLEXT_H

//------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>

//------------------------------------------------------------------------------
// Windows' OpenGL headers are stuck at version 1.1,
// so we need to keep track what we are using and add
// missing definitions here.

#define GL_CLAMP_TO_EDGE                            0x812F
#define GL_MULTISAMPLE                              0x809D

//------------------------------------------------------------------------------
// EXT_framebuffer_object

#define GL_FRAMEBUFFER_EXT                          0x8D40
#define GL_RENDERBUFFER_EXT                         0x8D41

#define GL_STENCIL_INDEX1_EXT                       0x8D46
#define GL_STENCIL_INDEX4_EXT                       0x8D47
#define GL_STENCIL_INDEX8_EXT                       0x8D48
#define GL_STENCIL_INDEX16_EXT                      0x8D49

#define GL_RENDERBUFFER_WIDTH_EXT                   0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT                  0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT         0x8D44
#define GL_RENDERBUFFER_RED_SIZE_EXT                0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT              0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT               0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT              0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT              0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT            0x8D55

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT   0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT   0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT 0x8CD4

#define GL_COLOR_ATTACHMENT0_EXT                    0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT                    0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT                    0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT                    0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT                    0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT                    0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT                    0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT                    0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT                    0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT                    0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT                   0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT                   0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT                   0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT                   0x8CED
#define GL_COLOR_ATTACHMENT14_EXT                   0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT                   0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT                     0x8D00
#define GL_STENCIL_ATTACHMENT_EXT                   0x8D20

#define GL_FRAMEBUFFER_COMPLETE_EXT                 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT    0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT    0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT       0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT   0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT   0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT              0x8CDD

#define GL_FRAMEBUFFER_BINDING_EXT                  0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT                 0x8CA7
#define GL_MAX_COLOR_ATTACHMENTS_EXT                0x8CDF
#define GL_MAX_RENDERBUFFER_SIZE_EXT                0x84E8

#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT        0x0506

typedef GLboolean (*PFNGLISRENDERBUFFEREXTPROC)(GLuint renderbuffer);
typedef void (*PFNGLBINDRENDERBUFFEREXTPROC)(GLenum target, GLuint renderbuffer);
typedef void (*PFNGLDELETERENDERBUFFERSEXTPROC)(GLsizei n, GLuint const *renderbuffers);
typedef void (*PFNGLGENRENDERBUFFERSEXTPROC)(GLsizei n, GLuint *renderbuffers);
typedef void (*PFNGLRENDERBUFFERSTORAGEEXTPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (*PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)(GLenum target, GLenum pname, GLint *params);
typedef GLboolean (*PFNGLISFRAMEBUFFEREXTPROC)(GLuint framebuffer);
typedef void (*PFNGLBINDFRAMEBUFFEREXTPROC)(GLenum target, GLuint framebuffer);
typedef void (*PFNGLDELETEFRAMEBUFFERSEXTPROC)(GLsizei n, GLuint const *framebuffers);
typedef void (*PFNGLGENFRAMEBUFFERSEXTPROC)(GLsizei n, GLuint *framebuffers);
typedef GLenum (*PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)(GLenum target);
typedef void (*PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (*PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (*PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (*PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (*PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (*PFNGLGENERATEMIPMAPEXTPROC)(GLenum target);

//------------------------------------------------------------------------------

#endif // QU_GLEXT_H
