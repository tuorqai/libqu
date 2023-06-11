//------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <libqu.h>

//------------------------------------------------------------------------------

static bool loop(void)
{
    float t = qu_get_time_mediump();

    qu_clear(0xff202020);

    qu_draw_circle(256.f, 256.f, 100.f, 0xffff0000, 0);
    qu_draw_line(256.f, 256.f, 256.f + 100.f * cosf(t), 256.f + 100.f * sinf(t),
        0xffff0000);

    qu_present();

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "[libqu] hello-libqu",
        .display_width = 512,
        .display_height = 512,
    });

    qu_execute(loop);
}

//------------------------------------------------------------------------------
