#ifndef URL_PARSER_H
#define URL_PARSER_H
#include <string.h>
#include "wrappers.h"
#include <ctype.h>

typedef struct url_info {
    char *protocol;
    char *host;
    char *port;
    char *path;
} URL_INFO;

/* Functions prototypes */
URL_INFO *parse_url(URL_INFO *info, const char *url);
#endif