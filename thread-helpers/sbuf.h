#ifndef SBUF_H
#define SBUF_H
#include "../semaphore/semaphore.h"
#include "../proxy-helpers/wrappers.h"

#define NTHREADS 4
#define SBUFSIZE 16

typedef struct {
    int *buf;          /* Buffer array */         
    int n;             /* Maximum number of slots */
    int front;         /* buf[(front+1)%n] is first item */
    int rear;          /* buf[rear%n] is last item */
    sem_t mutex;       /* Protects accesses to buf */
    sem_t slots;       /* Counts available slots */
    sem_t items;       /* Counts available items */
} sbuf_t;

extern sbuf_t sbuf;    /* Shared buffer of connected descriptors */

/* sbuf functions prototypes */
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);

#endif