#include "thread.h"

/************************************************
 * Wrappers for Pthreads thread control functions
 ************************************************/
 /*
  * Pthread_create - Starts  a  new  thread  in the calling process.
  */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
		    void * (*routine)(void *), void *argp) 
{
    int rc;

    if ((rc = pthread_create(tidp, attrp, routine, argp)) != 0)
	    posix_error(rc, "Pthread_create error");
}

/*
 * Pthread_cancel - Sends a cancellation request to the thread
 *                  with id tid
 */
void Pthread_cancel(pthread_t tid) {
    int rc;

    if ((rc = pthread_cancel(tid)) != 0)
	    posix_error(rc, "Pthread_cancel error");
}

/*
 * Pthread_join - Make calling thread wait for termination of the thread.
 */
void Pthread_join(pthread_t tid, void **thread_return) {
    int rc;

    if ((rc = pthread_join(tid, thread_return)) != 0)
	    posix_error(rc, "Pthread_join error");
}

/*
 * Pthread_join - Indicate that the thread TH is never to be joined
 *                with PTHREAD_JOIN. The resources of TH will therefore
 *                be freed immediately when it terminates, instead of
 *                waiting for another thread to perform PTHREAD_JOIN on it.
 */
void Pthread_detach(pthread_t tid) {
    int rc;

    if ((rc = pthread_detach(tid)) != 0)
	    posix_error(rc, "Pthread_detach error");
}

/*
 * Pthread_exit - Terminates the calling thread.
 */
void Pthread_exit(void *retval) {
    pthread_exit(retval);
}

/*
 * Pthread_exit - Returns id of the calling thread.
 */
pthread_t Pthread_self(void) {
    return pthread_self();
}

/*
 * Pthread_once -  The first call to pthread_once() by any thread
 *                 in a process, with a given once_control, shall call the
 *                 init_routine with no arguments. Subsequent calls of
 *                 pthread_once() with the same  once_control  shall  not
 *                 call  the  init_routine.
 */
void Pthread_once(pthread_once_t *once_control, void (*init_function)()) {
    pthread_once(once_control, init_function);
}