#ifndef CACHE_H
#define CACHE_H
#include <stdio.h>
#include <stdlib.h>
#include "../constants.h"
#include "../proxy-helpers/wrappers.h"
#include "../semaphore/semaphore.h"

/* Global variables */
extern unsigned long cur_time;        /* Used to update timestamp */

typedef struct cache_line {
    char content[MAX_OBJECT_SIZE];    /* Stored web object */
    int length;                       /* Actual length of web object */
    unsigned long long timestamp;     /* To perform LRU */
    size_t hash;                      /* Hash value of the request header */
    int valid_bit;                    /* Valid bit */
} cache_line_t;

/* Helper functions prototypes */
static int get_write_idx(cache_line_t *cache);
static size_t hash_func(char *str);

/* Functions prototypes */
void cache_init(cache_line_t **cache_p);
void write_cache(cache_line_t *cache, char *cache_buf, int object_size, size_t hash);
int is_cached(cache_line_t *cache, size_t target);
size_t hash(char *HTTP_request);
#endif
