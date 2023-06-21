//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_MATH_H
#define QU_MATH_H

//------------------------------------------------------------------------------

#include "libqu.h"

//------------------------------------------------------------------------------

void libqu_mat4_identity(float *m);
void libqu_mat4_copy(float *dst, float *src);
void libqu_mat4_multiply(float *m, float const *n);
void libqu_mat4_ortho(float *m, float l, float r, float b, float t);
void libqu_mat4_translate(float *m, float x, float y, float z);
void libqu_mat4_scale(float *m, float x, float y, float z);
void libqu_mat4_rotate(float *m, float rad, float x, float y, float z);
void libqu_mat4_inverse(float *dst, float const *src);
qu_vec2f libqu_mat4_transform_point(float const *m, qu_vec2f p);

//------------------------------------------------------------------------------

#endif // QU_MATH_H

//------------------------------------------------------------------------------
