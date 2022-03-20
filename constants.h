#ifndef CONSTANTS_H
#define CONSTANTS_H
/* Misc constants */
#define	MAXLINE	 8192     /* Max text line length */
#define MAXBUF   1048576  /* Max I/O buffer size */

/* Max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define CACHE_LINE_NUM (MAX_CACHE_SIZE / MAX_OBJECT_SIZE)

#define LISTENQ  1024  /* Second argument to listen() */

#endif