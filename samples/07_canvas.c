//------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <libqu.h>

//------------------------------------------------------------------------------

#define OBJECT_SPEED            (6.f)

//------------------------------------------------------------------------------

static qu_font font;
static float tx = 256.f, dx = OBJECT_SPEED;

static bool loop(void)
{
    tx += dx;

    if (dx > 0.f) {
        if (tx > (256.f + 128.f)) {
            dx = -OBJECT_SPEED;
        }
    } else if (dx < 0.f) {
        if (tx < (256.f - 128.f)) {
            dx = OBJECT_SPEED;
        }
    }

    float t = qu_get_time_mediump();
    float u = 256.f * sinf(t * 3.f) + 256.f;

    qu_set_view(256.0f, 256.0f, 512.0f + u, 512.0f + u, t * 100.f);
    qu_clear(0xff202020);

    qu_draw_circle(256.f, 256.f, 192.f, 0xffe0e000, 0);
    qu_draw_rectangle(4.f, 4.f, 64.f, 64.f, 0xffe00000, 0);
    qu_draw_rectangle(444.f, 4.f, 64.f, 64.f, 0xffe00000, 0);
    qu_draw_rectangle(444.f, 444.f, 64.f, 64.f, 0xffe00000, 0);
    qu_draw_rectangle(4.f, 444.f, 64.f, 64.f, 0xffe00000, 0);
    qu_draw_circle(tx, 476.f, 8.f, 0xff00e0e0, 0);

    qu_present();

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "libquack sample: canvas",
        .display_width = 512,
        .display_height = 512,
        .screen_mode = QU_SCREEN_MODE_USE_CANVAS,
    });

    font = qu_load_font("assets/sansation.ttf", 16.f);

    qu_execute(loop);

    return 0;
}

//------------------------------------------------------------------------------
