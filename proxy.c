#include "service.h"


int main(int argc, char *argv[])
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    cache_line_t *cache = NULL;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* Ignore SIGPIPE signal */
    Signal(SIGPIPE, SIG_IGN);

    listenfd = Open_listenfd(argv[1]);
    printf("Proxy listening on port: %s\n", argv[1]);

    /* Initialize cache */
    cache_init(&cache);

    /* Initialize semaphores */
    semaphore_init();

    sbuf_init(&sbuf, SBUFSIZE);
    for (int i = 0; i < NTHREADS; ++i)  /* Create worker threads */
        Pthread_create(&tid, NULL, thread, cache);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen,
                    hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        ++cur_time;        /* Update time */
        sbuf_insert(&sbuf, connfd);
    }
    Free(cache);
    Close(listenfd);
    exit(0);
}



