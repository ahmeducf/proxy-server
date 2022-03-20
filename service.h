#ifndef PROXY_H
#define PROXY_H

#include "constants.h"
#include "proxy_helpers/url_parser.h"
#include "proxy_helpers/wrappers.h"
#include "socket.h"
#include "robust_io.h"
#include "thread.h"
#include "thread_helpers/sbuf.h"
#include "thread_helpers/semaphore.h"
#include "cache.h"

/* Worker thread routine prototype */
void *thread(void *vargp);

/* Proxy/client and proxy/server functions prototypes */
static void serve_client(int connfd, cache_line_t *cache);
static void service_from_cache(int connfd, cache_line_t *cache, size_t matched_line_idx);
static void service_from_server(int connfd, URL_INFO url_info, char *parsed_request,
                                cache_line_t *cache, size_t HTTP_request_hash);
static int connect_server(URL_INFO url_info, char *parsed_request, int connfd, char *cache_buf);

/* Manipulating HTTP requests functions prototypes */
static int read_HTTP_request(int connfd, URL_INFO *url_infop, char *headers);
static int read_request_line(rio_t *rp, URL_INFO *url_infop, char *parsed_request);
static void read_request_headers(rio_t *rp, char *headers, URL_INFO *url_infop);
static void check_important_headers(char *header, int *important_headers_flag);

/* Error handling function prototypes */
static void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
#endif