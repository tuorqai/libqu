//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_TEXT_H
#define QU_TEXT_H

//------------------------------------------------------------------------------

#include "qu_graphics.h"
#include "qu_fs.h"

//------------------------------------------------------------------------------

void libqu_initialize_text(libqu_graphics *graphics);
void libqu_terminate_text(void);
int32_t libqu_load_font(libqu_file *file, float pt);
void libqu_delete_font(int32_t font_id);
void libqu_draw_text(int32_t font_id, float x, float y, qu_color color, char const *text);

//------------------------------------------------------------------------------

#endif // QU_TEXT_H

//------------------------------------------------------------------------------
