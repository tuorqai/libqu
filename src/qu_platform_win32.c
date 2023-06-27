
//------------------------------------------------------------------------------

#include "qu.h"

//------------------------------------------------------------------------------

#define THREAD_FLAG_WAIT            0x01
#define THREAD_FLAG_DETACHED        0x02

//------------------------------------------------------------------------------

struct libqu_thread
{
    DWORD id;
    HANDLE handle;
    CRITICAL_SECTION cs;
    UINT flags;
    char const *name;
    libqu_thread_func func;
    void *arg;
};

struct libqu_mutex
{
    CRITICAL_SECTION cs;
};

//------------------------------------------------------------------------------

static double      frequency_highp;
static double      start_highp;
static float       start_mediump;

//------------------------------------------------------------------------------

void libqu_platform_initialize(void)
{
    LARGE_INTEGER perf_clock_frequency, perf_clock_count;

    QueryPerformanceFrequency(&perf_clock_frequency);
    QueryPerformanceCounter(&perf_clock_count);

    frequency_highp = (double) perf_clock_frequency.QuadPart;
    start_highp = (double) perf_clock_count.QuadPart / frequency_highp;
    start_mediump = (float) GetTickCount() / 1000.f;
}

void libqu_platform_terminate(void)
{
}

//------------------------------------------------------------------------------
// Clock

float libqu_get_time_mediump(void)
{
    float seconds = (float) GetTickCount() / 1000.f;
    return seconds - start_mediump;
}

double libqu_get_time_highp(void)
{
    LARGE_INTEGER perf_clock_counter;
    QueryPerformanceCounter(&perf_clock_counter);

    double seconds = (double) perf_clock_counter.QuadPart / frequency_highp;
    return seconds - start_highp;
}

//------------------------------------------------------------------------------
// Threads

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

libqu_thread *libqu_create_thread(char const *name, libqu_thread_func func, void *arg)
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
