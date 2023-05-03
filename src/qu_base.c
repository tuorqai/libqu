//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include "qu_base.h"
#include "qu_gateway.h"

#if defined(__EMSCRIPTEN__)
#   include <emscripten.h>
#endif

//------------------------------------------------------------------------------

void qu_initialize(qu_params const *params)
{
    libqu_initialize(params);
}

void qu_terminate(void)
{
    libqu_terminate();
}

bool qu_process(void)
{
    return libqu_process();
}

#if defined(__EMSCRIPTEN__)

static void main_loop(void *callback_pointer)
{
    if (libqu_process()) {
        qu_loop_fn loop_fn = callback_pointer;
        loop_fn();
    } else {
        libqu_terminate();
        emscripten_cancel_main_loop();
    }
}

void qu_execute(qu_loop_fn loop_fn)
{
    emscripten_set_main_loop_arg(main_loop, loop_fn, 0, 1);
    exit(EXIT_SUCCESS);
}

#else

void qu_execute(qu_loop_fn loop_fn)
{
    while (libqu_process() && loop_fn()) {
        // Intentionally left blank
    }

    libqu_terminate();
    exit(EXIT_SUCCESS);
}

#endif

void qu_present(void)
{
    libqu_present();
}

//------------------------------------------------------------------------------