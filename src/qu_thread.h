//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_THREAD_H
#define QU_THREAD_H

//------------------------------------------------------------------------------

#include <stdint.h>

//------------------------------------------------------------------------------

typedef struct libqu_thread libqu_thread;
typedef struct libqu_mutex libqu_mutex;

//------------------------------------------------------------------------------

libqu_thread *libqu_create_thread(char const *name, intptr_t (*func)(void *), void *arg);
void libqu_detach_thread(libqu_thread *thread);
intptr_t libqu_wait_thread(libqu_thread *thread);

libqu_mutex *libqu_create_mutex(void);
void libqu_destroy_mutex(libqu_mutex *mutex);
void libqu_lock_mutex(libqu_mutex *mutex);
void libqu_unlock_mutex(libqu_mutex *mutex);

void libqu_sleep(double seconds);

//------------------------------------------------------------------------------

#endif // QU_THREAD_H

//------------------------------------------------------------------------------
