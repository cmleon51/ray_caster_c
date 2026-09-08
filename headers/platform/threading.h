#ifndef THREADING_H
#define THREADING_H

typedef void* Semaphore;
typedef void* AtomicInt;
typedef void* Thread;

typedef int (*ThreadFunction)(void *data);

int thread_get_num_logical_cpu_cores();

AtomicInt thread_create_atomic_int();
void thread_set_atomic_int(AtomicInt atomic_int, int value);
int thread_get_atomic_int(AtomicInt atomic_int);
void thread_destroy_atomic_int(AtomicInt atomic_int);

Semaphore thread_create_semaphore(unsigned int initial_value);
void thread_wait_semaphore(Semaphore semaphore);
void thread_signal_semaphore(Semaphore semaphore);
void thread_destroy_semaphore(Semaphore semaphore);

Thread thread_create(ThreadFunction function, const char *name, void *data);
void thread_wait(Thread thread);



#endif // THREADING_H
