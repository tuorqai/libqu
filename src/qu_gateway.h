//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_GATEWAY_H
#define QU_GATEWAY_H

//------------------------------------------------------------------------------

#include "qu_core.h"

//------------------------------------------------------------------------------

bool libqu_gl_check_extension(char const *name);
void *libqu_gl_proc_address(char const *name);
void libqu_notify_gc_created(libqu_gc gc);
void libqu_notify_gc_destroyed(void);

void libqu_notify_display_resize(int width, int height);

//------------------------------------------------------------------------------

#endif // QU_GATEWAY_H

//------------------------------------------------------------------------------