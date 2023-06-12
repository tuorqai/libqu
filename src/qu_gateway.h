//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_GATEWAY_H
#define QU_GATEWAY_H

//------------------------------------------------------------------------------

#include "qu_core.h"

//------------------------------------------------------------------------------

void libqu_initialize(qu_params const *params);
void libqu_terminate(void);
bool libqu_process(void);
void libqu_present(void);

bool libqu_gl_check_extension(char const *name);
void *libqu_gl_proc_address(char const *name);
void libqu_notify_gc_created(libqu_gc gc);
void libqu_notify_gc_destroyed(void);

void libqu_notify_display_resize(int width, int height);

//------------------------------------------------------------------------------

#endif // QU_GATEWAY_H

//------------------------------------------------------------------------------