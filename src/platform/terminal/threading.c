#include <platform/threading.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    pthread_t handle;
    ThreadFunction function;
    void *data;
} ThreadHandle;

static void *thread_start(void *arg) {
    ThreadHandle *thread = (ThreadHandle *)arg;
    thread->function(thread->data);

    return NULL;
}

int thread_get_num_logical_cpu_cores() {
    long cores = sysconf(_SC_NPROCESSORS_ONLN);

    return cores > 0 ? (int)cores : 1;
}

AtomicInt thread_create_atomic_int() {
    return (AtomicInt)calloc(1, sizeof(atomic_int));
}

void thread_set_atomic_int(AtomicInt atomic, int value) {
    atomic_store((atomic_int *)atomic, value);
}

int thread_get_atomic_int(AtomicInt atomic) {
    return atomic_load((atomic_int *)atomic);
}

void thread_destroy_atomic_int(AtomicInt atomic) {
    free(atomic);
}

Semaphore thread_create_semaphore(unsigned int initial_value) {
    sem_t *semaphore = malloc(sizeof(sem_t));

    if (semaphore && sem_init(semaphore, 0, initial_value) != 0) {
        free(semaphore);
        return NULL;
    }

    return (Semaphore)semaphore;
}

void thread_wait_semaphore(Semaphore semaphore) {
    while (sem_wait((sem_t *)semaphore) != 0 && errno == EINTR) {
    }
}

void thread_signal_semaphore(Semaphore semaphore) {
    sem_post((sem_t *)semaphore);
}

void thread_destroy_semaphore(Semaphore semaphore) {
    sem_destroy((sem_t *)semaphore);
    free(semaphore);
}

Thread thread_create(ThreadFunction function, const char *name, void *data) {
    (void)name;

    ThreadHandle *thread = malloc(sizeof(ThreadHandle));

    if (!thread)
        return NULL;

    thread->function = function;
    thread->data = data;

    if (pthread_create(&thread->handle, NULL, thread_start, thread) != 0) {
        free(thread);
        return NULL;
    }

    return (Thread)thread;
}

void thread_wait(Thread thread) {
    ThreadHandle *handle = (ThreadHandle *)thread;

    pthread_join(handle->handle, NULL);
    free(handle);
}
