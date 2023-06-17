
#ifndef QU_PLATFORM_H
#define QU_PLATFORM_H

//------------------------------------------------------------------------------

#include <stdint.h>

#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

//------------------------------------------------------------------------------

typedef struct libqu_thread libqu_thread;
typedef struct libqu_mutex libqu_mutex;
typedef intptr_t (*libqu_thread_func)(void *);

//------------------------------------------------------------------------------

void libqu_platform_initialize(void);
void libqu_platform_terminate(void);

float libqu_get_time_mediump(void);
double libqu_get_time_highp(void);

libqu_thread *libqu_create_thread(char const *name, libqu_thread_func func, void *arg);
void libqu_detach_thread(libqu_thread *thread);
intptr_t libqu_wait_thread(libqu_thread *thread);

libqu_mutex *libqu_create_mutex(void);
void libqu_destroy_mutex(libqu_mutex *mutex);
void libqu_lock_mutex(libqu_mutex *mutex);
void libqu_unlock_mutex(libqu_mutex *mutex);

void libqu_sleep(double seconds);

//------------------------------------------------------------------------------

#endif // QU_PLATFORM_H
