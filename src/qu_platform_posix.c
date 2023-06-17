
//------------------------------------------------------------------------------

#ifndef _WIN32

//------------------------------------------------------------------------------

#if defined(__linux__)
#   define _GNU_SOURCE
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "qu_log.h"
#include "qu_platform.h"

//------------------------------------------------------------------------------

#define THREAD_NAME_LENGTH          256

//------------------------------------------------------------------------------

struct libqu_thread
{
    pthread_t id;
    char name[THREAD_NAME_LENGTH];
    intptr_t(*func)(void *arg);
    void *arg;
};

struct libqu_mutex
{
    pthread_mutex_t id;
};

//------------------------------------------------------------------------------

static uint64_t start_mediump;
static double start_highp;

//------------------------------------------------------------------------------

void libqu_platform_initialize(void)
{
    // (?) Initialize clock

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    start_mediump = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    start_highp = (double) ts.tv_sec + (ts.tv_nsec / 1.0e9);
}

void libqu_platform_terminate(void)
{
}

//------------------------------------------------------------------------------
// Clock

float libqu_get_time_mediump(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    unsigned long long msec = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    return (msec - start_mediump) / 1000.0f;
}

double libqu_get_time_highp(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (double) ts.tv_sec + (ts.tv_nsec / 1.0e9) - start_highp;
}

//------------------------------------------------------------------------------
// Threads

static void *thread_main(void *thread_ptr)
{
    libqu_thread *thread = thread_ptr;
    intptr_t retval = thread->func(thread->arg);
    free(thread);

    return (void *) retval;
}

libqu_thread *libqu_create_thread(char const *name, intptr_t(*func)(void *), void *arg)
{
    libqu_thread *thread = calloc(1, sizeof(libqu_thread));

    if (!thread) {
        libqu_error("Failed to allocate memory for thread \'%s\'.\n", name);
        return NULL;
    }

    if (!name || name[0] == '\0') {
        strncpy(thread->name, "(unnamed)", THREAD_NAME_LENGTH);
    } else {
        strncpy(thread->name, name, THREAD_NAME_LENGTH);
    }

    thread->func = func;
    thread->arg = arg;

    int error = pthread_create(&thread->id, NULL, thread_main, thread);

    if (error) {
        libqu_error("Error (code %d) occured while attempting to create thread \'%s\'.\n", error, thread->name);
        free(thread);

        return NULL;
    }

#if defined(__linux__)
    pthread_setname_np(thread->id, thread->name);
#endif

    return thread;
}

void libqu_detach_thread(libqu_thread *thread)
{
    int error = pthread_detach(thread->id);

    if (error) {
        libqu_error("Failed to detach thread \'%s\', error code: %d.\n", thread->name, error);
    }
}

intptr_t libqu_wait_thread(libqu_thread *thread)
{
    void *retval;
    int error = pthread_join(thread->id, &retval);

    if (error) {
        libqu_error("Failed to join thread \'%s\', error code: %d.\n", thread->name, error);
    }

    return (intptr_t) retval;
}

libqu_mutex *libqu_create_mutex(void)
{
    libqu_mutex *mutex = calloc(1, sizeof(libqu_mutex));

    if (!mutex) {
        return NULL;
    }

    int error = pthread_mutex_init(&mutex->id, NULL);

    if (error) {
        libqu_error("Failed to create mutex, error code: %d.\n", error);
        return NULL;
    }

    return mutex;
}

void libqu_destroy_mutex(libqu_mutex *mutex)
{
    if (!mutex) {
        return;
    }

    int error = pthread_mutex_destroy(&mutex->id);

    if (error) {
        libqu_error("Failed to destroy mutex, error code: %d.\n", error);
    }

    free(mutex);
}

void libqu_lock_mutex(libqu_mutex *mutex)
{
    pthread_mutex_lock(&mutex->id);
}

void libqu_unlock_mutex(libqu_mutex *mutex)
{
    pthread_mutex_unlock(&mutex->id);
}

void libqu_sleep(double seconds)
{
    uint64_t s = (uint64_t) floor(seconds);

    struct timespec ts = {
        .tv_sec = s,
        .tv_nsec = (uint64_t) ((seconds - s) * 1000000000),
    };

    while ((nanosleep(&ts, &ts) == -1) && (errno == EINTR)) {
        // Wait, do nothing.
    }
}

//------------------------------------------------------------------------------

#endif // _WIN32
