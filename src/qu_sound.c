//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vorbis/vorbisfile.h>
#include "qu_log.h"
#include "qu_sound.h"

//------------------------------------------------------------------------------

struct sound_data
{
    int64_t (*read)(libqu_sound *sound, int16_t *dst, int64_t max_samples);
    void (*seek)(libqu_sound *sound, int64_t sample_offset);
    void (*close)(libqu_sound *sound);

    union {
        struct {
            int16_t bytes_per_sample;
            int64_t data_start;
            int64_t data_end;
        } wav;

        OggVorbis_File vorbis;
    };
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

            data->wav.bytes_per_sample = fmt.bits_per_sample / 8;
            sound->num_channels = fmt.num_channels;
            sound->sample_rate = fmt.sample_rate;
        } else if (strncmp("data", subchunk.id, 4) == 0) {
            sound->num_samples = subchunk.size / data->wav.bytes_per_sample;
            data->wav.data_start = libqu_ftell(sound->file);
            data->wav.data_end = data->wav.data_start + subchunk.size;

            data_found = true;
        }

        if (libqu_fseek(sound->file, subchunk_start + subchunk.size, SEEK_SET) == -1) {
            return -1;
        }
    }

    libqu_fseek(sound->file, data->wav.data_start, SEEK_SET);

    return 0;
}

static int64_t read_wav(libqu_sound *sound, int16_t *samples, int64_t max_samples)
{
    struct sound_data *data = sound->data;
    int16_t bytes_per_sample = data->wav.bytes_per_sample;
    int64_t samples_read = 0;

    while (samples_read < max_samples) {
        int64_t position = libqu_ftell(sound->file);

        if (position >= data->wav.data_end) {
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
    int64_t offset = data->wav.data_start + (sample_offset * data->wav.bytes_per_sample);
    libqu_fseek(sound->file, offset, SEEK_SET);
}

static void close_wav(libqu_sound *sound)
{
}

//------------------------------------------------------------------------------
// OGG reader [TODO: implement]

static char const *ogg_err(int status)
{
    if (status >= 0) {
        return "(no error)";
    }

    switch (status) {
    case OV_HOLE:           return "OV_HOLE";
    case OV_EREAD:          return "OV_EREAD";
    case OV_EFAULT:         return "OV_EFAULT";
    case OV_EINVAL:         return "OV_EINVAL";
    case OV_ENOTVORBIS:     return "OV_ENOTVORBIS";
    case OV_EBADHEADER:     return "OV_EBADHEADER";
    case OV_EVERSION:       return "OV_EVERSION";
    case OV_EBADLINK:       return "OV_EBADLINK";
    }

    return "(unknown error)";
}

static size_t vorbis_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    libqu_file *file = (libqu_file *) datasource;
    return libqu_fread(ptr, size * nmemb, file);
}

static int vorbis_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    libqu_file *file = (libqu_file *) datasource;
    return (int) libqu_fseek(file, offset, whence);
}

static long vorbis_tell_func(void *datasource)
{
    libqu_file *file = (libqu_file *) datasource;
    return (long) libqu_ftell(file);
}

static int64_t open_ogg(libqu_sound *sound)
{
    struct sound_data *data = sound->data;

    libqu_fseek(sound->file, 0, SEEK_SET);

    int test = ov_test_callbacks(
        sound->file,
        &data->vorbis,
        NULL, 0,
        (ov_callbacks) {
            .read_func = vorbis_read_func,
            .seek_func = vorbis_seek_func,
            .close_func = NULL,
            .tell_func = vorbis_tell_func,
        }
    );

    if (test < 0) {
        return -1;
    }

    int status = ov_test_open(&data->vorbis);

    if (status < 0) {
        libqu_error("Failed to open Ogg Vorbis media: %s\n", ogg_err(status));
        return -1;
    }

    vorbis_info *info = ov_info(&data->vorbis, -1);
    ogg_int64_t samples_per_channel = ov_pcm_total(&data->vorbis, -1);

    sound->num_channels = info->channels;
    sound->num_samples = samples_per_channel * info->channels;
    sound->sample_rate = info->rate;

    return 0;
}

static int64_t read_ogg(libqu_sound *sound, int16_t *samples, int64_t max_samples)
{
    struct sound_data *data = sound->data;
    long samples_read = 0;

    while (samples_read < max_samples) {
        int bytes_left = (max_samples - samples_read) / sizeof(int16_t);
        long bytes_read = ov_read(&data->vorbis, (char *) samples, bytes_left, 0, 2, 1, NULL);

        // End of file.
        if (bytes_read == 0) {
            break;
        }

        // Some error occured.
        if (bytes_read < 0) {
            libqu_error("Failed to read Ogg Vorbis from file %s. Reason: %s\n",
                libqu_file_repr(sound->file), ogg_err(bytes_read));
            break;
        }

        samples_read += bytes_read / sizeof(int16_t);
        samples += bytes_read / sizeof(int16_t);
    }

    return samples_read;
}

static void seek_ogg(libqu_sound *sound, int64_t sample_offset)
{
    struct sound_data *data = sound->data;
    ov_pcm_seek(&data->vorbis, sample_offset / sound->num_channels);
}

static void close_ogg(libqu_sound *sound)
{
    struct sound_data *data = sound->data;
    ov_clear(&data->vorbis);
}

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

        if (open_ogg(sound) == 0) {
            data->close = close_ogg;
            data->read = read_ogg;
            data->seek = seek_ogg;

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
