//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "qu_sound.h"

//------------------------------------------------------------------------------

struct sound_data
{
    int64_t (*read)(libqu_sound *sound, int16_t *dst, int64_t max_samples);
    void (*seek)(libqu_sound *sound, int64_t sample_offset);
    void (*close)(libqu_sound *sound);

    // WAV data

    int16_t wav_bytes_per_sample;
    int64_t wav_data_start;
    int64_t wav_data_end;
};

//------------------------------------------------------------------------------
// WAV reader

struct wav_chunk
{
    char id[4];
    uint32_t size;
    char format[4];
};

struct wav_subchunk
{
    char id[4];
    uint32_t size;
};

struct wav_fmt
{
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

static int64_t open_wav(libqu_sound *sound)
{
    libqu_fseek(sound->file, 0, SEEK_SET);

    struct wav_chunk chunk;

    if (libqu_fread(&chunk, sizeof(chunk), sound->file) < (int64_t) sizeof(chunk)) {
        return -1;
    }

    if (strncmp("RIFF", chunk.id, 4) || strncmp("WAVE", chunk.format, 4)) {
        return -1;
    }

    bool data_found = false;
    struct sound_data *data = sound->data;

    while (!data_found) {
        struct wav_subchunk subchunk;

        if (libqu_fread(&subchunk, sizeof(subchunk), sound->file) < (int64_t) sizeof(subchunk)) {
            return -1;
        }

        int64_t subchunk_start = libqu_ftell(sound->file);

        if (strncmp("fmt ", subchunk.id, 4) == 0) {
            struct wav_fmt fmt;

            if (libqu_fread(&fmt, sizeof(fmt), sound->file) < (int64_t) sizeof(fmt)) {
                return -1;
            }

            data->wav_bytes_per_sample = fmt.bits_per_sample / 8;
            sound->num_channels = fmt.num_channels;
            sound->sample_rate = fmt.sample_rate;
        } else if (strncmp("data", subchunk.id, 4) == 0) {
            sound->num_samples = subchunk.size / data->wav_bytes_per_sample;
            data->wav_data_start = libqu_ftell(sound->file);
            data->wav_data_end = data->wav_data_start + subchunk.size;

            data_found = true;
        }

        if (libqu_fseek(sound->file, subchunk_start + subchunk.size, SEEK_SET) == -1) {
            return -1;
        }
    }

    libqu_fseek(sound->file, data->wav_data_start, SEEK_SET);

    return 0;
}

static int64_t read_wav(libqu_sound *sound, int16_t *samples, int64_t max_samples)
{
    struct sound_data *data = sound->data;
    int16_t bytes_per_sample = data->wav_bytes_per_sample;
    int64_t samples_read = 0;

    while (samples_read < max_samples) {
        int64_t position = libqu_ftell(sound->file);

        if (position >= data->wav_data_end) {
            break;
        }

        unsigned char bytes[4];

        if (libqu_fread(bytes, bytes_per_sample, sound->file) != bytes_per_sample) {
            break;
        }

        switch (bytes_per_sample) {
        case 1:
            /* Unsigned 8-bit PCM */
            *samples++ = (((short) bytes[0]) - 128) << 8;
            break;
        case 2:
            /* Signed 16-bit PCM */
            *samples++ = (bytes[1] << 8) | bytes[0];
            break;
        case 3:
            /* Signed 24-bit PCM */
            *samples++ = (bytes[2] << 8) | bytes[1];
            break;
        case 4:
            /* Signed 32-bit PCM */
            *samples++ = (bytes[3] << 8) | bytes[2];
            break;
        }

        samples_read++;
    }

    return samples_read;
}

static void seek_wav(libqu_sound *sound, int64_t sample_offset)
{
    struct sound_data *data = sound->data;
    int64_t offset = data->wav_data_start + (sample_offset * data->wav_bytes_per_sample);
    libqu_fseek(sound->file, offset, SEEK_SET);
}

static void close_wav(libqu_sound *sound)
{
}

//------------------------------------------------------------------------------
// OGG reader [TODO: implement]

//------------------------------------------------------------------------------

libqu_sound *libqu_open_sound(libqu_file *file)
{
    libqu_sound *sound = calloc(1, sizeof(libqu_sound));
    struct sound_data *data = calloc(1, sizeof(struct sound_data));

    if (sound && data) {
        sound->file = file;
        sound->data = data;

        if (open_wav(sound) == 0) {
            data->close = close_wav;
            data->read = read_wav;
            data->seek = seek_wav;

            return sound;
        }
    }

    free(sound);
    free(data);

    return NULL;
}

void libqu_close_sound(libqu_sound *sound)
{
    if (!sound) {
        return;
    }

    ((struct sound_data *) sound->data)->close(sound);

    free(sound->data);
    free(sound);
}

int64_t libqu_read_sound(libqu_sound *sound, int16_t *samples, int64_t max_samples)
{
    return ((struct sound_data *) sound->data)->read(sound, samples, max_samples);
}

void libqu_seek_sound(libqu_sound *sound, int64_t sample_offset)
{
    ((struct sound_data *) sound->data)->seek(sound, sample_offset);
}

//------------------------------------------------------------------------------
