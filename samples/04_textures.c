//------------------------------------------------------------------------------

#include <libqu.h>

//------------------------------------------------------------------------------

static qu_texture texture;
static int frame;
static float update_time;

static bool loop(void)
{
    qu_clear(0xFF080820);
    qu_draw_subtexture(texture,
        128.f, 128.f, 256.f, 256.f,
        48.f * frame, 144.f, 48.f, 48.f);
    qu_present();

    float t = qu_get_time_mediump();

    if (t > update_time) {
        frame = (frame + 1) % 8;
        update_time = t + 0.1;
    }

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "libquack sample: textures",
        .display_width = 512,
        .display_height = 512,
        .screen_mode = QU_SCREEN_MODE_USE_CANVAS,
    });

    texture = qu_load_texture("assets/character.png");

    qu_execute(loop);
    return 0;
}

//------------------------------------------------------------------------------
