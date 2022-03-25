#include "socket.h"

/**************************** 
 * Sockets interface wrappers
 ****************************/

/*
 * Socket - Wrapper for socket, Create a new socket of type TYPE
 *          in domain DOMAIN, using protocol PROTOCOL.
 */
int Socket(int domain, int type, int protocol) 
{
    int rc;

    if ((rc = socket(domain, type, protocol)) < 0)
	    unix_error("Socket error");
    return rc;
}

/*
 * Setsockopt - manipulate  options for the socket referred
 *              to by the file descriptor sockfd.
 */
void Setsockopt(int s, int level, int optname, const void *optval, int optlen) 
{
    if (setsockopt(s, level, optname, optval, optlen) < 0)
	    unix_error("Setsockopt error");
}

/*
 * Bind - Wrapper for bind, asks the kernel to associate the
 *        server's socket address in my_addr with socket
 *        descriptor sockfd.
 */
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen) 
{
    if (bind(sockfd, my_addr, addrlen) < 0)
	    unix_error("Bind error");
}

/*
 * Listen - Wrapper for listen, marks  the  socket referred
 *          to by sockfd as a passive socket, that is, as a
 *          socket that will be used to  accept incoming
 *          connection requests using accept function.
 */
void Listen(int s, int backlog) 
{
    if (listen(s,  backlog) < 0)
	    unix_error("Listen error");
}

/*
 * Accept - Wrapper for accept, It extracts the first connection
 *          request on the queue of pending connections for the
 *          listening socket, sockfd, creates a new connected socket,
 *          and returns a new file descriptor referring to that socket.
 */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) 
{
    int rc;

    if ((rc = accept(s, addr, addrlen)) < 0)
	    unix_error("Accept error");
    return rc;
}

/*
 * Connect - Wrapper for connect, connects the socket referred to by the file
 *           descriptor sockfd to the address specified by addr. The addrlen
 *           argument specifies the size of addr.
 */
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen) 
{
    if (connect(sockfd, serv_addr, addrlen) < 0)
	    unix_error("Connect error");
}



/*******************************
 * Protocol-independent wrappers
 *******************************/

/*
 * Getaddrinfo - Wrapper for getaddrinfo, Given node and service, which identify
 *               an Internet host and a  service, getaddrinfo() returns one or
 *               more addrinfo structures, each of which contains an Internet address
 *               that can be specified in a call to bind() or connect().
 */
void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res)
{
    int rc;

    if ((rc = getaddrinfo(node, service, hints, res)) != 0) 
        gai_error(rc, "Getaddrinfo error");
}

/*
 * Getnameinfo - Wrapper for getnameinfo, The getnameinfo() function is the inverse of
 *               getaddrinfo():  it  converts a socket address to a corresponding
 *               host and service, in a protocol-independent manner.
 */
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, 
                 size_t hostlen, char *serv, size_t servlen, int flags)
{
    int rc;

    if ((rc = getnameinfo(sa, salen, host, hostlen, serv, 
                          servlen, flags)) != 0) 
        gai_error(rc, "Getnameinfo error");
}

/*
 * Freeaddrinfo - Wrapper for freeaddrinfo.
 */
void Freeaddrinfo(struct addrinfo *res)
{
    freeaddrinfo(res);
}

/*
 * Inet-ntop - Wrapper for inet_ntop, This function converts the
 *             network address structure src in the af address family
 *             into a character string. The resulting string is copied
 *             to the buffer pointed to by dst, which must be a non-null pointer.
 */
void Inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (!inet_ntop(af, src, dst, size))
        unix_error("Inet_ntop error");
}

/*
 * Inet-pton - Wrapper for inet_pton, This function converts the
 *             character string src into a network address structure
 *             in the af address family, then copies the network address
 *             structure to dst. The af argument must be either
 *             AF_INET or AF_INET6. dst is written in network byte order.
 */
void Inet_pton(int af, const char *src, void *dst) 
{
    int rc;

    rc = inet_pton(af, src, dst);
    if (rc == 0)
	    app_error("inet_pton error: invalid dotted-decimal address");
    else if (rc < 0)
        unix_error("Inet_pton error");
}




/******************************** 
 * Client/server helper functions
 ********************************/
/*
 * open_clientfd - Open connection to server at <hostname, port> and
 *     return a socket descriptor ready for reading and writing. This
 *     function is reentrant and protocol-independent.
 *
 *     On error, returns: 
 *       -2 for getaddrinfo error
 *       -1 with errno set for other errors.
 */
int open_clientfd(char *hostname, char *port) {
    int clientfd, rc;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV;  /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections */
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }
  
    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) 
            break; /* Success */
        if (close(clientfd) < 0) { /* Connect failed, try another */  //line:netp:openclientfd:closefd
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        } 
    } 

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}

/*  
 * open_listenfd - Open and return a listening socket on port. This
 *     function is reentrant and protocol-independent.
 *
 *     On error, returns: 
 *       -2 for getaddrinfo error
 *       -1 with errno set for other errors.
 */
int open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, rc, optval=1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -2;
    }

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;  /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        if (close(listenfd) < 0) { /* Bind failed, try the next */
            fprintf(stderr, "open_listenfd close failed: %s\n", strerror(errno));
            return -1;
        }
    }


    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
	return -1;
    }
    return listenfd;
}

/****************************************************
 * Wrappers for reentrant protocol-independent helpers
 ****************************************************/

/*
 * Open_clientfd - Wrapper for open_clientfd Open connection to server at <hostname, port> and
 *     return a socket descriptor ready for reading and writing. This
 *     function is reentrant and protocol-independent.
 */
int Open_clientfd(char *hostname, char *port) 
{
    int rc;

    if ((rc = open_clientfd(hostname, port)) < 0) 
	    unix_error("Open_clientfd error");
    return rc;
}

/*
 * Open_listenfd - Wrapper for open_listenfd, Open and return a listening socket on port. This
 *     function is reentrant and protocol-independent.
 */
int Open_listenfd(char *port) 
{
    int rc;

    if ((rc = open_listenfd(port)) < 0)
	    unix_error("Open_listenfd error");
    return rc;
}