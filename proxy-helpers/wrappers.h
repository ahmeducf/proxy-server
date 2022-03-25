#ifndef WRAPPERS_H
#define WRAPPERS_H
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "error_handling.h"

/* Signal wrappers functions prototypes */
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/* Unix I/O wrappers functions prototypes */
int Open(const char *pathname, int flags, mode_t mode);
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, const void *buf, size_t count);
off_t Lseek(int fildes, off_t offset, int whence);
void Close(int fd);
int Select(int  n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, 
	   struct timeval *timeout);

/* Dynamic storage allocation wrappers functions prototypes */
void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size);
void *Calloc(size_t nmemb, size_t size);
void Free(void *ptr);
#endif