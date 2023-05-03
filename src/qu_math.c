//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <math.h>
#include <string.h>
#include "qu_math.h"

//------------------------------------------------------------------------------

void libqu_mat4_identity(float *m)
{
    static float const identity[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    memcpy(m, identity, 16 * sizeof(float));
}

void libqu_mat4_copy(float *dst, float *src)
{
    memcpy(dst, src, 16 * sizeof(float));
}

void libqu_mat4_multiply(float *m, float const *n)
{
    float const result[] = {
        m[ 0] * n[ 0] + m[ 4] * n[ 1] + m[ 8] * n[ 2] + m[12] * n[ 3],
        m[ 1] * n[ 0] + m[ 5] * n[ 1] + m[ 9] * n[ 2] + m[13] * n[ 3],
        m[ 2] * n[ 0] + m[ 6] * n[ 1] + m[10] * n[ 2] + m[14] * n[ 3],
        m[ 3] * n[ 0] + m[ 7] * n[ 1] + m[11] * n[ 2] + m[15] * n[ 3],
        m[ 0] * n[ 4] + m[ 4] * n[ 5] + m[ 8] * n[ 6] + m[12] * n[ 7],
        m[ 1] * n[ 4] + m[ 5] * n[ 5] + m[ 9] * n[ 6] + m[13] * n[ 7],
        m[ 2] * n[ 4] + m[ 6] * n[ 5] + m[10] * n[ 6] + m[14] * n[ 7],
        m[ 3] * n[ 4] + m[ 7] * n[ 5] + m[11] * n[ 6] + m[15] * n[ 7],
        m[ 0] * n[ 8] + m[ 4] * n[ 9] + m[ 8] * n[10] + m[12] * n[11],
        m[ 1] * n[ 8] + m[ 5] * n[ 9] + m[ 9] * n[10] + m[13] * n[11],
        m[ 2] * n[ 8] + m[ 6] * n[ 9] + m[10] * n[10] + m[14] * n[11],
        m[ 3] * n[ 8] + m[ 7] * n[ 9] + m[11] * n[10] + m[15] * n[11],
        m[ 0] * n[12] + m[ 4] * n[13] + m[ 8] * n[14] + m[12] * n[15],
        m[ 1] * n[12] + m[ 5] * n[13] + m[ 9] * n[14] + m[13] * n[15],
        m[ 2] * n[12] + m[ 6] * n[13] + m[10] * n[14] + m[14] * n[15],
        m[ 3] * n[12] + m[ 7] * n[13] + m[11] * n[14] + m[15] * n[15],
    };

    memcpy(m, result, 16 * sizeof(float));
}

void libqu_mat4_ortho(float *m, float l, float r, float b, float t)
{
    float n = -1.f;
    float f = +1.f;

    m[ 0] = 2.0f / (r - l);
    m[ 1] = 0.0f;
    m[ 2] = 0.0f;
    m[ 3] = 0.0f;

    m[ 4] = 0.0f;
    m[ 5] = 2.0f / (t - b);
    m[ 6] = 0.0f;
    m[ 7] = 0.0f;

    m[ 8] = 0.0f;
    m[ 9] = 0.0f;
    m[10] = -2.0f / (f - n);
    m[11] = 0.0f;

    m[12] = -(r + l) / (r - l);
    m[13] = -(t + b) / (t - b);
    m[14] = -(f + n) / (f - n);
    m[15] = 1.0f;
}

void libqu_mat4_translate(float *m, float x, float y, float z)
{
    float const translation[] = {
        1.f,    0.f,    0.f,    0.f,
        0.f,    1.f,    0.f,    0.f,
        0.f,    0.f,    1.f,    0.f,
        x,      y,      z,      1.f,
    };

    libqu_mat4_multiply(m, translation);
}

void libqu_mat4_rotate(float *m, float rad, float x, float y, float z)
{
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;

    float c = cosf(rad);
    float s = sinf(rad);
    float ci = 1.0f - c;

    float xs = x * s;
    float ys = y * s;
    float zs = z * s;

    float const rotation[] = {
        xx * ci + c,    y  * ci + zs,   xz * ci - ys,   0.f,
        xy * ci - zs,   yy * ci + c,    yz * ci + xs,   0.f,
        xz * ci + ys,   yz * ci - xs,   zz * ci + c,    0.f,
        0.f,            0.f,            0.f,            1.f,
    };

    libqu_mat4_multiply(m, rotation);
}

void libqu_mat4_inverse(float *dst, float const *src)
{
    float det =
        src[0] * (src[15] * src[5] - src[13] * src[7]) -
        src[4] * (src[15] * src[1] - src[13] * src[3]) +
        src[8] * (src[ 7] * src[1] - src[ 5] * src[3]);

    if (det == 0.f) {
        libqu_mat4_identity(dst);
        return;
    }

    dst[ 0] = +(src[15] * src[5] - src[13] * src[7]) / det;
    dst[ 1] = -(src[15] * src[4] - src[12] * src[7]) / det;
    dst[ 2] = 0.0f;
    dst[ 3] = +(src[13] * src[4] - src[12] * src[5]) / det;
    dst[ 4] = -(src[15] * src[1] - src[13] * src[3]) / det;
    dst[ 5] = +(src[15] * src[0] - src[12] * src[3]) / det;
    dst[ 6] = 0.0f;
    dst[ 7] = -(src[13] * src[0] - src[12] * src[1]) / det;
    dst[ 8] = 0.0f;
    dst[ 9] = 0.0f;
    dst[10] = 1.0f;
    dst[11] = 0.0f;
    dst[12] = +(src[7] * src[1] - src[5] * src[3]) / det;
    dst[13] = -(src[7] * src[0] - src[4] * src[3]) / det;
    dst[14] = 0.0f;
    dst[15] = +(src[5] * src[0] - src[4] * src[1]) / det;
}

qu_vec2f libqu_mat4_transform_point(float const *m, qu_vec2f p)
{
    return (qu_vec2f) {
        .x = m[0] * p.x + m[1] * p.y + m[3],
        .y = m[4] * p.x + m[5] * p.y + m[7],
    };
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
