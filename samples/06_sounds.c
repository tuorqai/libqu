//------------------------------------------------------------------------------

#include <stdio.h>
#include "libquack/libquack.h"

//------------------------------------------------------------------------------

static qu_sound coin;
static qu_music music;
static qu_stream music_stream;

static void on_key_pressed(qu_key key)
{
    if (key == QU_KEY_Z) {
        qu_pause_stream(music_stream);
        return;
    }

    if (key == QU_KEY_X) {
        qu_unpause_stream(music_stream);
        return;
    }

    qu_stream stream = qu_play_sound(coin);

    printf("stream id: 0x%08x (%d)\n", stream.id, stream.id);
}

static bool loop(void)
{
    qu_clear(0xFF080820);
    qu_present();

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "libquack sample: sounds",
        .display_width = 512,
        .display_height = 512,
    });

    coin = qu_load_sound("assets/coin.wav");

    if (!coin.id) {
        qu_terminate();
        return 0;
    }

    music = qu_open_music("assets/balda.wav");

    if (!music.id) {
        qu_terminate();
        return 0;
    }

    music_stream = qu_loop_music(music);

    qu_on_key_pressed(on_key_pressed);
    qu_on_key_repeated(on_key_pressed);
    qu_execute(loop);

    return 0;
}

//------------------------------------------------------------------------------
