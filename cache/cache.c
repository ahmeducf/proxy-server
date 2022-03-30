#include "cache.h"

unsigned long cur_time = 0;     /* Used to update timestamp */

/* Helper functions prototypes */
static int get_write_idx(cache_line_t *cache);
static size_t hash_func(char *str);



/*
 * cache_init - initialize proxy's cache by allocating
 *              memory for it and initializing it to zeros.
 */
void cache_init(cache_line_t **cache_p) {
    int i;
    cache_line_t *p;

    *cache_p = Calloc(CACHE_LINE_NUM, sizeof(cache_line_t));
    for (p = *cache_p, i = 0; i < CACHE_LINE_NUM; ++i, ++p) {
        p->length = 0;
        p->timestamp = 0;
        p->hash = 0;
        p->valid_bit = 0;
    }
}

/*
 * is_cached - Check if a web object target is cached. It searches cache by
 *             checking valid bit and hash value. Return the index of cache
 *             line if there is a match. Otherwise, return -1.
 *             If there is a cache hit, the timestamp in the corresponding
 *             cache line would be updated.
 */
int is_cached(cache_line_t *cache, size_t target) {
    int i;
    cache_line_t *p;
    for (p = cache, i = 0; i < CACHE_LINE_NUM; ++i, ++p) {
        if (p->valid_bit == 1 && p->hash == target) {
            P(&mutex_writing_cache_line[i]);
            p->timestamp = cur_time;
            V(&mutex_writing_cache_line[i]);
            return i;
        }

    }
    return -1;
}

/*
 * get_write_index - Helper function searches the cache for an empty cache
 *                   line. if found one returns its index. otherwise,
 *                   returns the index of least-recent-used line.
 */
static int get_write_idx(cache_line_t *cache) {
    int i;
    cache_line_t *p;
    unsigned long lru_time = __UINT64_MAX__;
    int lru_idx;

    for (p = cache, i = 0; i < CACHE_LINE_NUM; ++i, ++p) {
        if (p->valid_bit == 0) {
            return i;
        } else {
            if (p->timestamp < lru_time) {
                lru_time = p->timestamp;
                lru_idx = i;
            }
        }
    }
    return lru_idx;
}

/*
 * write_cache - Search for write index in the cache and write cache_buf in the
 *               cache line at this index. it uses mutex semaphore to lock
 *               writing at this cache line from other threads.
 */
void write_cache(cache_line_t *cache, char *cache_buf, int object_size, size_t hash)
{
    int write_idx = get_write_idx(cache);

    P(&mutex_writing_cache_line[write_idx]);
    memcpy((cache + write_idx)->content, cache_buf, object_size);
    (cache + write_idx)->length = object_size;
    (cache + write_idx)->timestamp = cur_time;
    (cache + write_idx)->hash = hash;
    (cache + write_idx)->valid_bit = 1;
    V(&mutex_writing_cache_line[write_idx]);
}

/*
 * hash_func - Helper function generates hash value from string
 */
static size_t hash_func(char *str) {
    unsigned long res = 5381;
    int c;

    while ((c = *(str++)))
        res = ((res << 5) + res) + c; /* hash * 33 + c */

    return res;
}

/*
 * hash - Generate hash value for HTTP request
 */
size_t hash(char *HTTP_request)
{
    return hash_func(HTTP_request);
}