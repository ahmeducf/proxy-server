#include "service.h"

#define CACHE_USED


static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* Proxy/client and proxy/server functions prototypes */
static void serve_client(int connfd, cache_line_t *cache);
static void service_from_cache(int connfd, cache_line_t *cache, size_t matched_line_idx);
static void service_from_server(int connfd, URL_INFO *url_info, char *parsed_request,
                                cache_line_t *cache, size_t HTTP_request_hash);
static int connect_server(URL_INFO *url_info, char *parsed_request, int connfd, char *cache_buf);

/* Manipulating HTTP requests functions prototypes */
static int read_HTTP_request(int connfd, URL_INFO *url_infop, char *headers);
static int read_request_line(rio_t *rp, URL_INFO *url_infop, char *parsed_request);
static void read_request_headers(rio_t *rp, char *headers, URL_INFO *url_infop);
static void check_important_headers(char *header, int *important_headers_flag);

/* Error handling function prototypes */
static void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);


                 
/*
 * thread - Represent the routine of worker threads. It waits until it's able to
 *          remove a connected descriptor from the buffer and then calls serve_client
 *          function. The thread runs in detached mode to be reaped automatically
 *          by the kernel after finishing.
 */
void *thread(void *vargp)
{
    Pthread_detach(pthread_self());
    cache_line_t *cache = (cache_line_t *)vargp;
    while (1) {
        int connfd = sbuf_remove(&sbuf);
        serve_client(connfd, cache);
        Close(connfd);
    }
}

/*
 * serve_client - Handle one HTTP request/response transaction. It reads
 *                the request from client. If the requested content is
 *                cached reads it from cache directly, otherwise connect server
 *                to get the content.
 */
static void serve_client(int connfd, cache_line_t *cache)
{
    URL_INFO *url_info = Malloc(4 * sizeof(char*));
    char parsed_request[MAXLINE];
    size_t HTTP_request_hash;
    int matched_line_idx;

    /* Read request line and headers from the client */
    if (read_HTTP_request(connfd, url_info, parsed_request) < 0)
        return;

    HTTP_request_hash = hash(parsed_request);
    printf("\n<-- Parsed request [hash = %lu] -->\n%s",
           HTTP_request_hash, parsed_request);

#ifdef CACHE_USED
    if ((matched_line_idx = is_cached(cache, HTTP_request_hash)) >= 0) {
        printf("<-- Cache Hit! -->\n");
        /* Read object from cache */
        printf("<-- service from cache -->\n");
        service_from_cache(connfd, cache, matched_line_idx);
        printf("This is a cached version of the request!\n");
    }
    else {
        printf("Cache miss!\n");
        /* Forward request on to the server */
        printf("<-- service from server -->\n");
        service_from_server(connfd, url_info, parsed_request,
                            cache, HTTP_request_hash);
    }
#else
    char *cache_buf[MAX_OBJECT_SIZE];
    /* Forward request on to the server */
    connect_server(url_info, parsed_request, connfd, cache_buf);
#endif
    Free(url_info);
}

/*
 * connect_server - Use url_info to connect with server, if connection success
 *                  send parsed_request to the server. then read server's
 *                  response and forward it to client. if proxy has a cache, copy
 *                  the response content to cache_buf to save it in the cache.
 */
static int connect_server(URL_INFO *url_info, char *parsed_request, int connfd, char *cache_buf)
{
    int clientfd;
    int total = 0;
    char *host, *port, buf[MAXBUF];
    rio_t rio;
    ssize_t nread;

    host = url_info->host;
    port = url_info->port;
    if (!host || !port) return 0;
    if ((clientfd = open_clientfd(host, port)) < 0) {
        char msg[MAXLINE];
        sprintf(msg, "Server lookup failure:\t(%s:%s), Name or service not known", host, port);
        printf("<-- Can't connect server -->\n");
        clienterror(connfd, "GET", "400", "Bad request", msg);
        return 0;
    }

    printf("Connection sent to host %s on port %s\n", host, port);

    Rio_readinitb(&rio, clientfd);
    /* Send request line and headers to the server */
    Rio_writen(clientfd, parsed_request, strlen(parsed_request));

    /* Read server's response and forward it to client */
    while ((nread = Rio_readnb(&rio, buf, MAXBUF)) > 0) {
        Rio_writen(connfd, buf, nread);
        Rio_writen(STDOUT_FILENO, buf, nread);
        // printf("%*.*s", (int)nread, (int)nread, buf);

#ifdef CACHE_USED
        if ((total + nread) < MAX_OBJECT_SIZE) {
            memcpy(cache_buf + total, buf, nread);
            total += nread;
        }
#endif
    }
    Close(clientfd);
    return total;
}

/*
 * read_request_line - read HTTP request line and parse
 *         url into url_info struct
 */
static int read_request_line(rio_t *rp, URL_INFO *url_infop, char *parsed_request)
{
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];

    /* Read request line*/
    if (Rio_readlineb(rp, buf, MAXLINE) == 0){
        return -1;
    }
    printf("%s", buf);
    /* Parse request line*/
    sscanf(buf, "%s %s %s", method, url, version);
    // Check request method
    if (strcasecmp(method, "GET")) {
        clienterror(rp->rio_fd, method, "501", "Not Implemented",
                    "Our proxy does not implement this method");
        return -1;
    }
    // Check HTTP version
    if (strcasecmp("HTTP/1.0", version) && strcasecmp("HTTP/1.1", version)) {
        clienterror(rp->rio_fd, version, "400", "Bad request",
                    "Invalid HTTP version");
        return -1;
    } else {
        strcpy(version, "HTTP/1.0");
    }
    parse_url(url_infop, url);
    if (!url_infop) {
        clienterror(rp->rio_fd, url, "400", "Bad request",
                    "Invalid URI");
        return -1;
    }

    sprintf(parsed_request, "%s %s %s\r\n", method, url_infop->path, version);
    return 0;
}

/*
 * read_HTTP_request - read HTTP line and headers from client
 *      and return url_info and manipulated request headers
 *      ready to be sent to the server
 */
static int read_HTTP_request(int connfd, URL_INFO *url_infop, char *parsed_request)
{
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    if (read_request_line(&rio, url_infop, parsed_request) < 0)
        return -1;

    read_request_headers(&rio, parsed_request, url_infop);
    return 0;
}

/*
 * check_important_headers - Helper function check for
 *      important headers [Host,User-Agent,Connection,Proxy-Connection]
 */
static void check_important_headers(char *header, int *important_headers_flag)
{
    char buf[MAXLINE];  /* Copy of the header line */
    strcpy(buf, header);

    char *p = strchr(buf, ':');
    if (!p) return;

    *p = '\0';
    char header_title[MAXLINE];
    strcpy(header_title, buf);

    if (!strcasecmp(header_title, "Host"))
        important_headers_flag[0] = 1;
    else if (!strcasecmp(header_title, "User-Agent"))
        important_headers_flag[1] = 1;
    else if (!strcasecmp(header_title, "Connection")) {
        important_headers_flag[2] = 1;
        strcpy(header, "Connection: close\r\n");        /* Always send connection: close */
    }
    else if (!strcasecmp(header_title, "Proxy-Connection")) {
        important_headers_flag[3] = 1;
        strcpy(header, "Proxy-Connection: close\r\n");  /* Always send Proxy-Connection: close */
    }
}

/*
 * read_request_headers - read HTTP request headers from the client
 *      and manipulate them to be sent to the server. It makes sure
 *      to append [Host,User-Agent,Connection,Proxy-Connection]
 *      headers if not sent with the request headers from the client
 */
static void read_request_headers(rio_t *rp, char* parsed_request, URL_INFO *url_infoP)
{
    char buf[MAXLINE];
    /* [0]: Host    [1]: User-Agent     [2]: Connection     [3]: Proxy-Connection */
    int important_headers_flag[5] = {0,0,0,0,0};
    char header[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    check_important_headers(buf, important_headers_flag);
    while(strcmp(buf, "\r\n")) {
        strcat(parsed_request, buf);
        Rio_readlineb(rp, buf, MAXLINE);
        check_important_headers(buf, important_headers_flag);
    }

    /* Append important headers if not sent from the client */
    for (int i = 0; i < 4; i++) {
        if (!important_headers_flag[i]) {
            switch (i)
            {
                case 0:
                {
                    sprintf(header, "Host: ");
                    sprintf(header, "%s%s\r\n", header, url_infoP->host);
                    strcat(parsed_request, header);
                    break;
                }
                case 1:
                {
                    sprintf(header, "User-Agent: ");
                    sprintf(header, "%s%s", header, user_agent_hdr);
                    strcat(parsed_request, header);
                    break;
                }
                case 2:
                {
                    sprintf(header, "Connection: ");
                    sprintf(header, "%sclose\r\n", header);
                    strcat(parsed_request, header);
                    break;
                }
                case 3:
                {
                    sprintf(header, "Proxy-Connection: ");
                    sprintf(header, "%sclose\r\n", header);
                    strcat(parsed_request, header);
                    break;
                }
                default:
                    break;
            }
        }
    }
    strcat(parsed_request, buf);
}

/*
 * service_from_cache - Read requested object from cache. It allows readers to share
 *                      the object. use mutex semaphores to favor readers. No reader
 *                      be kept waiting unless a writer has already been granted
 *                      permission to use the object. once there is at least one
 *                      reader, lock cache line that contains the object for writing.
 *                      free it back when readers_cnt of the cache line become zero.
 */
static void service_from_cache(int connfd, cache_line_t *cache, size_t matched_line_idx)
{
    P(&mutex_readcnt[matched_line_idx]);
    cache_line_readcnt[matched_line_idx]++;
    if (cache_line_readcnt[matched_line_idx] == 1) {        /* First in */
        P(&mutex_writing_cache_line[matched_line_idx]);
    }
    V(&mutex_readcnt[matched_line_idx]);

    /* Read content from cahce */
    char *content = (cache + matched_line_idx)->content;
    int length = (cache + matched_line_idx)->length;
    printf("%s\n", content);
    Rio_writen(connfd, content, length);

    P(&mutex_readcnt[matched_line_idx]);
    cache_line_readcnt[matched_line_idx]--;
    if (cache_line_readcnt[matched_line_idx] == 0) {        /* Last out */
        V(&mutex_writing_cache_line[matched_line_idx]);
    }
    V(&mutex_readcnt[matched_line_idx]);
}

/*
 * service_from_server - connect server to send it parsed_request, read its
 *                       response, send it back to the client, save it in
 *                       a buffer. if object size is less than MAX_OBJECT_SIZE,
 *                       save object in the cache to read it quickly in case of
 *                       same future requests without connecting the server.
 */
static void service_from_server(int connfd, URL_INFO *url_info, char *parsed_request,
                         cache_line_t *cache, size_t HTTP_request_hash)
{
    char cache_buf[MAX_OBJECT_SIZE];
    printf("<-- service from server directly -->\n");
    int object_size = connect_server(url_info, parsed_request, connfd, cache_buf);
    /* Write object to cache */
    if (object_size < MAX_OBJECT_SIZE && object_size > 0) {
        write_cache(cache, cache_buf, object_size, HTTP_request_hash);
        printf("Web object has been written to cache!\n");
    }
    else {
        printf("object size is very large! can't write it to cache!\n");
    }
}

/*
 * clienterror - returns an error message to the client
 */
static void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>Web Proxy</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
