#include "semaphore.h"

int cache_line_readcnt[CACHE_LINE_NUM];           /* number of readers per cache line */
sem_t mutex_readcnt[CACHE_LINE_NUM];              /* Lock read_cnt per cache line */
sem_t mutex_writing_cache_line[CACHE_LINE_NUM];   /* lock cache_line while writing on it */



/*******************************
 * Wrappers for Posix semaphores
 *******************************/

/*
 * Sem_init - Wrapper for sem_init, initializes the unnamed semaphore
 *            at the address pointed to by sem. The value argument
 *            specifies the initial value for the semaphore.
 */
void Sem_init(sem_t *sem, int pshared, unsigned int value) 
{
    if (sem_init(sem, pshared, value) < 0)
	    unix_error("Sem_init error");
}

/*
 * P() - Wrapper for sem_wait(), decrements (locks) the semaphore pointed to by sem.
 *       If the semaphore's value is greater than zero, then the decrement proceeds,
 *       and the function returns, immediately. If the semaphore currently has
 *       the value zero, then the call blocks until either it becomes possible
 *       to perform the decrement (i.e., the semaphore value rises above zero),
 *       or a signal handler interrupts the call.
 */
void P(sem_t *sem) 
{
    if (sem_wait(sem) < 0)
	    unix_error("P error");
}

/*
 * V() - Wrapper for sem_post(), increments (unlocks) the semaphore
 *       pointed to by sem. If the semaphore's value consequently
 *       becomes greater than zero, then another process or thread
 *       blocked in a sem_wait() call will be woken up and
 *       proceed to lock the semaphore.
 */
void V(sem_t *sem) 
{
    if (sem_post(sem) < 0)
	    unix_error("V error");
}


/*
 * semaphore_init - Initialize cache_line_readcnt, mutex_readcnt and
 *                  mutex_writing_cache_line variables to zeros.
 */
void semaphore_init() {
    for (int i = 0; i < CACHE_LINE_NUM; ++i) {
        cache_line_readcnt[i] = 0;
    }

    for (int i = 0; i < CACHE_LINE_NUM; ++i) {
        Sem_init(&mutex_readcnt[i], 0, 1);
    }
    for (int i = 0; i < CACHE_LINE_NUM; ++i) {
        Sem_init(&mutex_writing_cache_line[i], 0, 1);
    }
}