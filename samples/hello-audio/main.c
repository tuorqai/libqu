//------------------------------------------------------------------------------

#include <stdio.h>
#include <libqu.h>

//------------------------------------------------------------------------------

#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF

//------------------------------------------------------------------------------

static qu_font font;
static qu_sound sound;
static qu_music music[3];
static qu_stream streams[3];

static void toggle_music(int n)
{
    qu_stream s = qu_loop_music(music[n]);

    if (s.id == streams[n].id) {
        qu_stop_stream(streams[n]);
        streams[n].id = 0;
    } else {
        streams[n] = s;
    }
}

static void on_key_pressed(qu_key key)
{
    switch (key) {
    case QU_KEY_1:
        toggle_music(0);
        break;
    case QU_KEY_2:
        toggle_music(1);
        break;
    case QU_KEY_3:
        toggle_music(2);
        break;
    case QU_KEY_SPACE:
        qu_play_sound(sound);
        break;
    default:
        break;
    }
}

static bool loop(void)
{
    qu_clear(0xFF000000);

    qu_draw_text(font, 30.f, 40.f, WHITE, "Track #1");
    qu_draw_text(font, 30.f, 80.f, WHITE, "Track #2");
    qu_draw_text(font, 30.f, 120.f, WHITE, "Track #3");

    qu_draw_text_fmt(font, 150.f, 40.f, WHITE, "0x%08x", streams[0]);
    qu_draw_text_fmt(font, 150.f, 80.f, WHITE, "0x%08x", streams[1]);
    qu_draw_text_fmt(font, 150.f, 120.f, WHITE, "0x%08x", streams[2]);

    qu_draw_text_fmt(font, 320.f, 40.f, WHITE, "%d", streams[0]);
    qu_draw_text_fmt(font, 320.f, 80.f, WHITE, "%d", streams[1]);
    qu_draw_text_fmt(font, 320.f, 120.f, WHITE, "%d", streams[2]);

    qu_present();

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "[libqu] hello-audio",
        .display_width = 512,
        .display_height = 512,
    });

    font = qu_load_font("assets/font.ttf", 18);
    sound = qu_load_sound("assets/coin.wav");
    music[0] = qu_open_music("assets/dungeon.ogg");
    music[1] = qu_open_music("assets/overworld.ogg");
    music[2] = qu_open_music("assets/ostrich.ogg");

    qu_on_key_pressed(on_key_pressed);
    qu_execute(loop);

    return 0;
}
