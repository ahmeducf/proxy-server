#ifndef SOCKET_H
#define SOCKET_H
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../constants.h"
#include "../proxy-helpers/error_handling.h"

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

/* Reentrant protocol-independent client/server helpers prototypes */
int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);

/* Wrappers for reentrant protocol-independent client/server helpers prototypes */
int Open_clientfd(char *hostname, char *port);
int Open_listenfd(char *port);

/* Sockets interface wrappers prototypes */
int Socket(int domain, int type, int protocol);
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen);
void Listen(int s, int backlog);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

/* Protocol independent wrappers prototypes */
void Getaddrinfo(const char *node, const char *service, 
                 const struct addrinfo *hints, struct addrinfo **res);
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, 
                 size_t hostlen, char *serv, size_t servlen, int flags);
void Freeaddrinfo(struct addrinfo *res);
void Inet_ntop(int af, const void *src, char *dst, socklen_t size);
void Inet_pton(int af, const char *src, void *dst);
#endif