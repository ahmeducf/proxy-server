#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include "../constants.h"

/* error-handling functions prototypes */
void unix_error(char *msg);
void posix_error(int code, char *msg);
void gai_error(int code, char *msg);
void app_error(char *msg);

#endif