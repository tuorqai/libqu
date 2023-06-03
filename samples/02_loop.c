//------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <libqu.h>

//------------------------------------------------------------------------------

static qu_font font;

static bool loop(void)
{
    float t = qu_get_time_mediump();

    qu_clear(0xff202020);

    qu_draw_circle(256.f, 256.f, 100.f, 0xffff0000, 0);
    qu_draw_line(256.f, 256.f, 256.f + 100.f * cosf(t), 256.f + 100.f * sinf(t),
        0xffff0000);
    qu_draw_text_fmt(font, 16.f, 440.f, 0xffffffff, "t = %.2f", t);

    qu_present();

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "libquack sample: loop",
        .display_width = 512,
        .display_height = 512,
        .screen_mode = QU_SCREEN_MODE_UPDATE_VIEW,
    });

    font = qu_load_font("assets/sansation.ttf", 16.f);

    qu_execute(loop);

    return 0;
}

//------------------------------------------------------------------------------
