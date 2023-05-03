//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include "qu_audio.h"
#include "qu_log.h"

//------------------------------------------------------------------------------

static void initialize(qu_params const *params)
{
    libqu_info("Null audio module initialized.\n");
}

static void terminate(void)
{
    libqu_info("Null audio module terminated.\n");
}

static bool is_initialized(void)
{
    return true;
}

static int32_t load_sound(libqu_file *file)
{
    return 1;
}

static void delete_sound(int32_t sound_id)
{
}

static int32_t play_sound(int32_t sound_id)
{
    return 1;
}

static int32_t loop_sound(int32_t sound_id)
{
    return 1;
}

static int32_t open_music(libqu_file *file)
{
    return 1;
}

static void close_music(int32_t music_id)
{
}

static int32_t play_music(int32_t music_id)
{
    return 1;
}

static int32_t loop_music(int32_t music_id)
{
    return 1;
}

static void pause_stream(int32_t stream_id)
{
}

static void unpause_stream(int32_t stream_id)
{
}

static void stop_stream(int32_t stream_id)
{
}

//------------------------------------------------------------------------------

void libqu_construct_null_audio(libqu_audio *audio)
{
    *audio = (libqu_audio) {
        .initialize = initialize,
        .terminate = terminate,
        .is_initialized = is_initialized,
        .load_sound = load_sound,
        .delete_sound = delete_sound,
        .play_sound = play_sound,
        .loop_sound = loop_sound,
        .open_music = open_music,
        .close_music = close_music,
        .play_music = play_music,
        .loop_music = loop_music,
        .pause_stream = pause_stream,
        .unpause_stream = unpause_stream,
        .stop_stream = stop_stream,
    };
}

//------------------------------------------------------------------------------
