//------------------------------------------------------------------------------

#include <stdlib.h>
#include <libqu.h>

//------------------------------------------------------------------------------

static bool loop(void)
{
    qu_clear(0xFF202020);

    for (int i = 0; i < 100; i++) {
        float x = (float) (rand() % 512);
        float y = (float) (rand() % 512);

        qu_draw_point(x, y, 0xFFFFFFFF);
    }

    qu_draw_line(0.f, 20.f, 512.f, 20.f, 0xFFFFFFFF);
    qu_draw_line(0.f, 492.f, 512.f, 492.f, 0xFFFFFFFF);

    qu_draw_rectangle(8.f, 8.f, 64.f, 64.f, 0xFF00FF00, 0x00000000);
    qu_draw_rectangle(440.f, 8.f, 64.f, 64.f, 0xFFFFFF00, 0x800000FF);

    qu_draw_triangle(256.f, 128.f, 128.f, 384.f, 384.f, 384.f, 0xFF000000, 0xFF404040);

    qu_present();

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "libquack sample: primitives",
        .display_width = 512,
        .display_height = 512,
    });

    qu_execute(loop);
    return 0;
}

//------------------------------------------------------------------------------
