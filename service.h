#ifndef PROXY_H
#define PROXY_H

#include "constants.h"
#include "proxy-helpers/url_parser.h"
#include "proxy-helpers/wrappers.h"
#include "socket-interface/socket.h"
#include "robust-input-output/robust_io.h"
#include "thread-helpers/thread.h"
#include "thread-helpers/sbuf.h"
#include "semaphore/semaphore.h"
#include "cache/cache.h"

/* Worker thread routine prototype */
void *thread(void *vargp);
#endif