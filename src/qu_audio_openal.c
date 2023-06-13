//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#if defined(__EMSCRIPTEN__)
#   include <AL/al.h>
#   include <AL/alc.h>
#else
#   include <al.h>
#   include <alc.h>
#endif

#include <string.h>
#include "qu_array.h"
#include "qu_audio.h"
#include "qu_halt.h"
#include "qu_log.h"
#include "qu_sound.h"
#include "qu_thread.h"

//------------------------------------------------------------------------------

enum
{
    MAX_STREAMS = 64,
};

enum stream_type
{
    STREAM_INACTIVE = 0,
    STREAM_STATIC,
    STREAM_DYNAMIC,
};

enum
{
    MUSIC_BUFFER_COUNT = 4,
    MUSIC_BUFFER_LENGTH = 8192,
};

struct sound
{
    ALenum format;
    ALuint buffer;
    int16_t *samples;
};

struct music
{
    libqu_file *file;
    libqu_sound *decoder;
    struct stream *stream;
};

struct stream
{
    enum stream_type type;
    ALuint source;
    int16_t gen;
    bool loop; // muzükağa ere tuttullar
    libqu_thread *thread; // muzükağa ere tuttullar
};

struct impl
{
    bool initialized;
    ALCdevice *device;
    ALCcontext *context;
    libqu_mutex *mutex;
    struct stream streams[MAX_STREAMS];
    libqu_array *sounds;
    libqu_array *music;
};

static struct impl impl;

//------------------------------------------------------------------------------

// Kanal aqsaanügar barsar formatü bierer.
static ALenum choose_format(int num_channels)
{
    switch (num_channels) {
    case 1:
        return AL_FORMAT_MONO16;
    case 2:
        return AL_FORMAT_STEREO16;
    }

    return AL_INVALID_ENUM;
}

// Indeksten uonna kölyönetten stream identifikatorün
// ongoron bierer.
static int32_t encode_stream_id(int index, int gen)
{
    return (gen << 16) | 0x0000CC00 | index;
}

// Tuttulla turar stream'i bosqolotor.
// Bolğomto: mutex qatanan turarün körö sürüt.
static void release_stream(struct stream *stream)
{
    ALint state;
    alGetSourcei(stream->source, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING) {
        alSourceStop(stream->source);
    }

    alSourcei(stream->source, AL_BUFFER, 0);

    stream->type = STREAM_INACTIVE;
    stream->gen = (stream->gen + 1) & 0x7FFF;
}

// Bosqo stream indeksin bulan bierer.
static int find_stream_index(void)
{
    // Bastaan tulaajaq stream'neri köröbyt.
    // Köstybeteğine oonnjoon toqtoobut stream'neri kördyybyt
    // uonna qattaan tuttabüt.
    for (int i = 0; i < MAX_STREAMS; i++) {
        struct stream *stream = &impl.streams[i];

        if (stream->type == STREAM_INACTIVE) {
            return i;
        } else if (stream->type == STREAM_STATIC) {
            ALint state;
            alGetSourcei(stream->source, AL_SOURCE_STATE, &state);

            if (state == AL_STOPPED) {
                release_stream(stream);
                return i;
            }
        }

        // Dinamiceskaj stream toqtootoğuna bejete STREAM_INACTIVE dien
        // köryngy ülünüaqtaaq!
    }

    // Bosqo stream suoq ebit.
    return -1;
}

// Staticeskaj stream'i sağalüür, ol ebeter sample'lere
// bary paamakka baarü.
static int32_t start_static_stream(int32_t sound_id, bool loop)
{
    struct sound *sound = libqu_array_get(impl.sounds, sound_id);

    if (!sound) {
        return 0;
    }

    int index = -1;
    int gen = -1;

    libqu_lock_mutex(impl.mutex);

    if ((index = find_stream_index()) != -1) {
        struct stream *stream = &impl.streams[index];

        stream->type = STREAM_STATIC;

        alSourcei(stream->source, AL_BUFFER, sound->buffer);
        alSourcei(stream->source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcePlay(stream->source);

        gen = stream->gen;
    }

    libqu_unlock_mutex(impl.mutex);

    return (index >= 0) ? encode_stream_id(index, gen) : 0;
}

// Identifikatortan stream objegün bulan bierer, tabüllübatağüna NULL.
static struct stream *get_stream_by_id(int32_t stream_id)
{
    if ((stream_id & 0x0000FF00) != 0x0000CC00) {
        return NULL;
    }

    int index = stream_id & 0xFF;
    int gen = (stream_id >> 16) & 0x7FFF;

    return (impl.streams[index].gen == gen) ? &impl.streams[index] : NULL;
}

static int fill_buffer(ALuint buffer, ALenum format, int16_t *samples,
                       int sample_rate, libqu_sound *decoder, bool loop)
{
    int64_t n = libqu_read_sound(decoder, samples, MUSIC_BUFFER_LENGTH);

    if (n == 0) {
        if (loop) {
            libqu_seek_sound(decoder, 0);
            n = libqu_read_sound(decoder, samples, MUSIC_BUFFER_LENGTH);

            if (n == 0) {
                return -1;
            }
        } else {
            return -2;
        }
    }

    alBufferData(buffer, format, samples, sizeof(short) * n, sample_rate);

    return 0;
}

static void clear_buffer_queue(ALuint source)
{
    ALint buffers_queued;
    alGetSourcei(source, AL_BUFFERS_QUEUED, &buffers_queued);

    for (int n = 0; n < buffers_queued; n++) {
        ALuint buffer;
        alSourceUnqueueBuffers(source, 1, &buffer);
    }
}

static intptr_t music_main(void *data)
{
    struct music *music = data;
    struct stream *stream = music->stream;

    ALenum format = choose_format(music->decoder->num_channels);
    int16_t *samples = malloc(sizeof(int16_t) * MUSIC_BUFFER_LENGTH);

    if (format == AL_INVALID_ENUM || !samples) {
        return 0;
    }

    alSourcei(stream->source, AL_BUFFER, 0);
    alSourcei(stream->source, AL_LOOPING, AL_FALSE);

    ALuint buffers[MUSIC_BUFFER_COUNT];
    alGenBuffers(MUSIC_BUFFER_COUNT, buffers);

    libqu_seek_sound(music->decoder, 0);
    clear_buffer_queue(stream->source);

    for (int i = 0; i < MUSIC_BUFFER_COUNT; i++) {
        fill_buffer(buffers[i], format, samples, music->decoder->sample_rate,
                    music->decoder, stream->loop);
        alSourceQueueBuffers(stream->source, 1, &buffers[i]);
    }

    alSourcePlay(stream->source);

    bool streaming = true;

    while (true) {
        bool paused = false;

        libqu_lock_mutex(impl.mutex);

        ALint state;
        alGetSourcei(stream->source, AL_SOURCE_STATE, &state);

        if (state == AL_PAUSED) {
            paused = true;
        } else if (state == AL_STOPPED) {
            streaming = false;
        }

        libqu_unlock_mutex(impl.mutex);

        if (paused) {
            libqu_sleep(0.1);
            continue;
        }

        if (!streaming) {
            break;
        }

        ALint buffers_processed;
        alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &buffers_processed);

        for (int i = 0; i < buffers_processed; i++) {
            ALuint buffer;
            alSourceUnqueueBuffers(stream->source, 1, &buffer);

            int status = fill_buffer(buffer, format, samples,
                                     music->decoder->sample_rate,
                                     music->decoder, stream->loop);

            if (status == -1) {
                break;
            }

            if (status == -2) {
                streaming = false;
                break;
            }

            alSourceQueueBuffers(stream->source, 1, &buffer);
        }

        libqu_sleep(0.1);
    }

    clear_buffer_queue(stream->source);
    alDeleteBuffers(MUSIC_BUFFER_COUNT, buffers);
    free(samples);

    libqu_lock_mutex(impl.mutex);
    stream->thread = false;
    libqu_unlock_mutex(impl.mutex);

    music->stream = NULL;

    return 0;
}

static int32_t start_dynamic_stream(int32_t music_id, bool loop)
{
    struct music *music = libqu_array_get(impl.music, music_id);

    if (!music) {
        return 0;
    }

    if (music->stream) {
        return 0;
    }

    int index = -1;
    int gen = -1;

    libqu_lock_mutex(impl.mutex);

    if ((index = find_stream_index()) != -1) {
        struct stream *stream = &impl.streams[index];

        music->stream = stream;
        stream->type = STREAM_DYNAMIC;
        stream->loop = loop;
        stream->thread = libqu_create_thread("music", music_main, music);

        gen = stream->gen;
    }

    libqu_unlock_mutex(impl.mutex);

    return (index >= 0) ? encode_stream_id(index, gen) : 0;
}

static void sound_dtor(void *data)
{
    struct sound *sound = data;

    libqu_lock_mutex(impl.mutex);

    for (int i = 0; i < MAX_STREAMS; i++) {
        struct stream *stream = &impl.streams[i];

        if (stream->type == STREAM_STATIC) {
            ALint buffer;
            alGetSourcei(stream->source, AL_BUFFER, &buffer);

            if ((ALuint) buffer == sound->buffer) {
                release_stream(stream);
            }
        }
    }

    libqu_unlock_mutex(impl.mutex);

    alDeleteBuffers(1, &sound->buffer);
    free(sound->samples);
}

static void music_dtor(void *data)
{
    struct music *music = data;

    if (music->stream) {
        libqu_lock_mutex(impl.mutex);
        release_stream(music->stream);
        libqu_unlock_mutex(impl.mutex);

        if (music->stream->thread) {
            libqu_wait_thread(music->stream->thread);
            music->stream->thread = NULL;
        }
    }

    libqu_close_sound(music->decoder);
    libqu_fclose(music->file);
}

//------------------------------------------------------------------------------

static void initialize(qu_params const *params)
{
    memset(&impl, 0, sizeof(impl));

    if (!(impl.mutex = libqu_create_mutex())) {
        libqu_error("Can't create mutex for OpenAL audio module.\n");
        return;
    }

    if (!(impl.sounds = libqu_create_array(sizeof(struct sound), sound_dtor))) {
        libqu_error("Can't allocate resource array for OpenAL.\n");
        return;
    }

    if (!(impl.music = libqu_create_array(sizeof(struct music), music_dtor))) {
        libqu_error("Can't allocate resource array for OpenAL.\n");
        return;
    }

    if (!(impl.device = alcOpenDevice(NULL))) {
        libqu_error("OpenAL: failed to open device.\n");
        return;
    }

    if (!(impl.context = alcCreateContext(impl.device, NULL))) {
        libqu_error("OpenAL: failed to create context.\n");
        return;
    }

    if (!alcMakeContextCurrent(impl.context)) {
        libqu_error("OpenAL: failed to activate context.\n");
        return;
    }

    ALuint sources[MAX_STREAMS];
    alGenSources(MAX_STREAMS, sources);

    for (int i = 0; i < MAX_STREAMS; i++) {
        impl.streams[i].source = sources[i];
    }

    libqu_info("OpenAL audio initialized.\n");
    impl.initialized = true;
}

static void terminate(void)
{
    libqu_destroy_array(impl.music);
    libqu_destroy_array(impl.sounds);

    if (impl.device) {
        if (impl.context) {
            for (int i = 0; i < MAX_STREAMS; i++) {
                alDeleteSources(1, &impl.streams[i].source);
            }

            alcMakeContextCurrent(NULL);
            alcDestroyContext(impl.context);
        }

        alcCloseDevice(impl.device);
    }

    libqu_destroy_mutex(impl.mutex);

    if (impl.initialized) {
        libqu_info("OpenAL audio terminated.\n");
        impl.initialized = false;
    }
}

static bool is_initialized(void)
{
    return impl.initialized;
}

static void set_master_volume(float volume)
{
    alListenerf(AL_GAIN, volume);
}

static int32_t load_sound(libqu_file *file)
{
    libqu_sound *decoder = libqu_open_sound(file);

    if (!decoder) {
        return 0;
    }

    struct sound sound = {
        .format = choose_format(decoder->num_channels),
        .samples = malloc(sizeof(int16_t) * decoder->num_samples),
    };

    int64_t buffer_size = decoder->num_samples * sizeof(int16_t);
    int64_t sample_rate = decoder->sample_rate;

    if (sound.format != AL_INVALID_ENUM && sound.samples) {
        int64_t required = decoder->num_samples;
        
        libqu_read_sound(decoder, sound.samples, required);
        libqu_close_sound(decoder);
    }

    alGenBuffers(1, &sound.buffer);
    alBufferData(sound.buffer, sound.format, sound.samples,
        buffer_size, sample_rate);

    return libqu_array_add(impl.sounds, &sound);
}

static void delete_sound(int32_t sound_id)
{
    libqu_array_remove(impl.sounds, sound_id);
}

static int32_t play_sound(int32_t sound_id)
{
    return start_static_stream(sound_id, false);
}

static int32_t loop_sound(int32_t sound_id)
{
    return start_static_stream(sound_id, true);
}

#ifdef __EMSCRIPTEN__

// Disable music playback in Emscripten until I find out
// why this pile of shit does zero effort to play
// those motherfucking queues. AL_STOPPED, my ass.
// Why the fuck did you stop? Doesn't even try to explain.
// No errors, no nothing *flips table*

static int32_t open_music_none(libqu_file *file)
{
    return 1;
}

static void close_music_none(int32_t music_id)
{
}

static int32_t play_music_none(int32_t music_id)
{
    return 0;
}

static int32_t loop_music_none(int32_t music_id)
{
    return 0;
}

#endif

static int32_t open_music(libqu_file *file)
{
    struct music music = {
        .file = file,
        .decoder = libqu_open_sound(file),
        .stream = NULL,
    };

    if (!music.decoder) {
        return 0;
    }

    return libqu_array_add(impl.music, &music);
}

static void close_music(int32_t music_id)
{
    libqu_array_remove(impl.music, music_id);
}

static int32_t play_music(int32_t music_id)
{
    return start_dynamic_stream(music_id, false);
}

static int32_t loop_music(int32_t music_id)
{
    return start_dynamic_stream(music_id, true);
}

static void pause_stream(int32_t stream_id)
{
    struct stream *stream = get_stream_by_id(stream_id);

    if (stream) {
        libqu_lock_mutex(impl.mutex);

        ALint state;
        alGetSourcei(stream->source, AL_SOURCE_STATE, &state);

        if (state == AL_PLAYING) {
            alSourcePause(stream->source);
        }
    
        libqu_unlock_mutex(impl.mutex);
    }
}

static void unpause_stream(int32_t stream_id)
{
    struct stream *stream = get_stream_by_id(stream_id);

    if (stream) {
        libqu_lock_mutex(impl.mutex);

        ALint state;
        alGetSourcei(stream->source, AL_SOURCE_STATE, &state);

        if (state == AL_PAUSED) {
            alSourcePlay(stream->source);
        }

        libqu_unlock_mutex(impl.mutex);
    }
}

static void stop_stream(int32_t stream_id)
{
    struct stream *stream = get_stream_by_id(stream_id);

    if (stream) {
        libqu_lock_mutex(impl.mutex);
        release_stream(stream);
        libqu_unlock_mutex(impl.mutex);
    }
}

//------------------------------------------------------------------------------

void libqu_construct_openal_audio(libqu_audio *audio)
{
    *audio = (libqu_audio) {
        .initialize = initialize,
        .terminate = terminate,
        .is_initialized = is_initialized,
        .set_master_volume = set_master_volume,
        .load_sound = load_sound,
        .delete_sound = delete_sound,
        .play_sound = play_sound,
        .loop_sound = loop_sound,
#ifdef __EMSCRIPTEN__
        .open_music = open_music_none,
        .close_music = close_music_none,
        .play_music = play_music_none,
        .loop_music = loop_music_none,
#else
        .open_music = open_music,
        .close_music = close_music,
        .play_music = play_music,
        .loop_music = loop_music,
#endif
        .pause_stream = pause_stream,
        .unpause_stream = unpause_stream,
        .stop_stream = stop_stream,
    };
}

//------------------------------------------------------------------------------
