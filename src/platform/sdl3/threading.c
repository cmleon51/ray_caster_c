#include <platform/threading.h>
#include <stdlib.h>

#include <SDL3/SDL_atomic.h>
#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>

int thread_get_num_logical_cpu_cores() {
    return SDL_GetNumLogicalCPUCores();
}

AtomicInt thread_create_atomic_int() {
    return (AtomicInt)calloc(1, sizeof(SDL_AtomicInt));
}

void thread_set_atomic_int(AtomicInt atomic_int, int value) {
    SDL_SetAtomicInt((SDL_AtomicInt*)atomic_int, value);
}

int thread_get_atomic_int(AtomicInt atomic_int) {
    return SDL_GetAtomicInt((SDL_AtomicInt*)atomic_int);
}

void thread_destroy_atomic_int(AtomicInt atomic_int) {
    free(atomic_int);
}

Semaphore thread_create_semaphore(unsigned int initial_value) {
    return (Semaphore)SDL_CreateSemaphore(initial_value);
}

void thread_wait_semaphore(Semaphore semaphore) {
    SDL_WaitSemaphore((SDL_Semaphore*)semaphore);
}

void thread_signal_semaphore(Semaphore semaphore) {
    SDL_SignalSemaphore((SDL_Semaphore*)semaphore);
}

void thread_destroy_semaphore(Semaphore semaphore) {
    SDL_DestroySemaphore((SDL_Semaphore*)semaphore);
}

Thread thread_create(ThreadFunction function, const char *name, void *data) {
    return (Thread)SDL_CreateThread((SDL_ThreadFunction)function, name, data);
}

void thread_wait(Thread thread) {
    SDL_WaitThread((SDL_Thread*)thread, NULL);
}
