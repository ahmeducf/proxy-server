#include "error_handling.h"



/************************** 
 * Error-handling functions
 **************************/
 /*
  * unix_error - Report error message for unix-functions errors.
  */
void unix_error(char *msg) /* Unix-style error */
{
    if (errno)
        fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

/*
 * posix_error - Report error message for posix-functions errors.
 */
void posix_error(int code, char *msg) /* Posix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
}

/*
 * gai_error - Report error message for getaddrinfo function errors.
 */
void gai_error(int code, char *msg) /* Getaddrinfo-style error */
{
    fprintf(stderr, "%s: %s\n", msg, gai_strerror(code));
}

/*
 * app_error - Report error messages.
 */
void app_error(char *msg) /* Application error */
{
    fprintf(stderr, "%s\n", msg);
}