//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_AUDIO_H
#define QU_AUDIO_H

//------------------------------------------------------------------------------

#include "libqu.h"
#include "qu_fs.h"

//------------------------------------------------------------------------------

typedef struct libqu_audio
{
    void (*initialize)(qu_params const *params);
    void (*terminate)(void);
    bool (*is_initialized)(void);

    void (*set_master_volume)(float volume);

    int32_t (*load_sound)(libqu_file *file);
    void (*delete_sound)(int32_t sound_id);
    int32_t (*play_sound)(int32_t sound_id);
    int32_t (*loop_sound)(int32_t sound_id);

    int32_t (*open_music)(libqu_file *file);
    void (*close_music)(int32_t music_id);
    int32_t (*play_music)(int32_t music_id);
    int32_t (*loop_music)(int32_t music_id);

    void (*pause_stream)(int32_t stream_id);
    void (*unpause_stream)(int32_t stream_id);
    void (*stop_stream)(int32_t stream_id);
} libqu_audio;

//------------------------------------------------------------------------------

void libqu_construct_null_audio(libqu_audio *audio);
void libqu_construct_openal_audio(libqu_audio *audio);

//------------------------------------------------------------------------------

#endif // QU_AUDIO_H

//------------------------------------------------------------------------------
