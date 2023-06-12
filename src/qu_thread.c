//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#if defined(__linux__)
#   define _GNU_SOURCE
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "qu_log.h"
#include "qu_thread.h"

//------------------------------------------------------------------------------

#define THREAD_NAME_LENGTH          256

//------------------------------------------------------------------------------
// Windows Threads

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define THREAD_FLAG_WAIT            0x01
#define THREAD_FLAG_DETACHED        0x02

struct libqu_thread
{
    DWORD id;
    HANDLE handle;
    CRITICAL_SECTION cs;
    UINT flags;
    char const *name;
    intptr_t (*func)(void *arg);
    void *arg;
};

struct libqu_mutex
{
    CRITICAL_SECTION cs;
};

static void thread_end(libqu_thread *thread)
{
    // If the thread is detached, then only its info struct should be freed.
    if (!(thread->flags & THREAD_FLAG_DETACHED)) {
        EnterCriticalSection(&thread->cs);

        // If the thread is being waited in wait_thread(), do not release
        // its resources.
        if (thread->flags & THREAD_FLAG_WAIT) {
            LeaveCriticalSection(&thread->cs);
            return;
        }

        LeaveCriticalSection(&thread->cs);

        CloseHandle(thread->handle);
        DeleteCriticalSection(&thread->cs);
    }

    HeapFree(GetProcessHeap(), 0, thread);
}

static DWORD WINAPI thread_main(LPVOID param)
{
    libqu_thread *thread = (libqu_thread *) param;
    intptr_t retval = thread->func(thread->arg);

    thread_end(thread);

    return retval;
}

libqu_thread *libqu_create_thread(char const *name, intptr_t (*func)(void *), void *arg)
{
    libqu_thread *thread = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(libqu_thread));

    thread->name = name;
    thread->func = func;
    thread->arg = arg;

    InitializeCriticalSection(&thread->cs);
    thread->flags = 0;

    thread->handle = CreateThread(NULL, 0, thread_main, thread, 0, &thread->id);

    if (!thread->handle) {
        HeapFree(GetProcessHeap(), 0, thread);
        return NULL;
    }

    return thread;
}

void libqu_detach_thread(libqu_thread *thread)
{
    EnterCriticalSection(&thread->cs);
    thread->flags |= THREAD_FLAG_DETACHED;
    LeaveCriticalSection(&thread->cs);

    CloseHandle(thread->handle);
    DeleteCriticalSection(&thread->cs);
}

intptr_t libqu_wait_thread(libqu_thread *thread)
{
    EnterCriticalSection(&thread->cs);

    if (thread->flags & THREAD_FLAG_DETACHED) {
        LeaveCriticalSection(&thread->cs);
        return -1;
    }

    thread->flags |= THREAD_FLAG_WAIT;
    LeaveCriticalSection(&thread->cs);

    DWORD retval;
    WaitForSingleObject(thread->handle, INFINITE);
    GetExitCodeThread(thread->handle, &retval);

    thread->flags &= ~THREAD_FLAG_WAIT;
    thread_end(thread);

    return (intptr_t) retval;
}

libqu_mutex *libqu_create_mutex(void)
{
    libqu_mutex *mutex = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(libqu_mutex));
    InitializeCriticalSection(&mutex->cs);

    return mutex;
}

void libqu_destroy_mutex(libqu_mutex *mutex)
{
    DeleteCriticalSection(&mutex->cs);
    HeapFree(GetProcessHeap(), 0, mutex);
}

void libqu_lock_mutex(libqu_mutex *mutex)
{
    EnterCriticalSection(&mutex->cs);
}

void libqu_unlock_mutex(libqu_mutex *mutex)
{
    LeaveCriticalSection(&mutex->cs);
}

void libqu_sleep(double seconds)
{
    DWORD milliseconds = (DWORD) (seconds * 1000);
    Sleep(milliseconds);
}

//------------------------------------------------------------------------------
// pthreads

#else

#include <errno.h>
#include <pthread.h>

struct libqu_thread
{
    pthread_t id;
    char name[THREAD_NAME_LENGTH];
    intptr_t (*func)(void *arg);
    void *arg;
};

struct libqu_mutex
{
    pthread_mutex_t id;
};

static void *thread_main(void *thread_ptr)
{
    libqu_thread *thread = thread_ptr;
    intptr_t retval = thread->func(thread->arg);
    free(thread);

    return (void *) retval;
}

libqu_thread *libqu_create_thread(char const *name, intptr_t (*func)(void *), void *arg)
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
        .tv_nsec = (uint64_t)((seconds - s) * 1000000000),
    };

    while ((nanosleep(&ts, &ts) == -1) && (errno == EINTR)) {
        // Wait, do nothing.
    }
}

#endif
