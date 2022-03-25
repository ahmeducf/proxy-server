#ifndef SEMAPHORE_H
#define SEMAPHORE_
#include <semaphore.h>
#include "../proxy-helpers/error_handling.h"
#include "../constants.h"

extern int cache_line_readcnt[CACHE_LINE_NUM];           /* number of readers per cache line */
extern sem_t mutex_readcnt[CACHE_LINE_NUM];              /* Lock read_cnt per cache line */
extern sem_t mutex_writing_cache_line[CACHE_LINE_NUM];   /* lock cache_line while writing on it */

/* POSIX semaphore wrappers */
void Sem_init(sem_t *sem, int pshared, unsigned int value);
void P(sem_t *sem);
void V(sem_t *sem);

/* Initialization function prototype */
void semaphore_init();
#endif