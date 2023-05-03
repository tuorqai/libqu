//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_SOUND_H
#define QU_SOUND_H

//------------------------------------------------------------------------------

#include "qu_fs.h"

//------------------------------------------------------------------------------

typedef struct libqu_sound
{
    int16_t num_channels;
    int64_t num_samples;
    int64_t sample_rate;
    libqu_file *file;
    void *data;
} libqu_sound;

//------------------------------------------------------------------------------

libqu_sound *libqu_open_sound(libqu_file *file);
void libqu_close_sound(libqu_sound *sound);
int64_t libqu_read_sound(libqu_sound *sound, int16_t *samples, int64_t max_samples);
void libqu_seek_sound(libqu_sound *sound, int64_t sample_offset);

//------------------------------------------------------------------------------

#endif // QU_SOUND_H

//------------------------------------------------------------------------------
