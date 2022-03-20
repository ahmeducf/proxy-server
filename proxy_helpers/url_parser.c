#include "url_parser.h"

/*
 * parse_url - get plain URL and parse it into
 *      [protocol,host,port,path] in a URL_INFO struct
 */
URL_INFO *parse_url(URL_INFO *info, const char *url)
{
    char *save_ptr;
    if (!info || !url)
        return NULL;

    info->protocol = __strtok_r(strcpy((char *)Malloc(strlen(url) + 1), url),
                                "://", &save_ptr);
    info->host = strstr(url, "://");

    if (info->host) {
        info->host += 3;
        char *host_port_path = strcpy((char *)Calloc(1, strlen(info->host) + 1),
                                      info->host);
        info->host = __strtok_r(host_port_path, ":", &save_ptr);
        info->host = __strtok_r(host_port_path, "/", &save_ptr);
    } else {
        char *host_port_path = strcpy((char *)Calloc(1, strlen(url) + 1), url);
        info->host = __strtok_r(host_port_path, ":", &save_ptr);
        info->host = __strtok_r(host_port_path, "/", &save_ptr);
    }
    char *URL = strcpy((char *)Malloc(strlen(url) + 1), url);
    info->port = strstr(URL + 6, ":");
    char *port_path = 0;
    char *port_path_copy = 0;
    if (info->port && isdigit(*(port_path = (char *)info->port + 1))) {
        port_path_copy = strcpy((char *)Malloc(strlen(port_path) + 1),
                                port_path);
        char *r = __strtok_r(port_path, "/", &save_ptr);
        if (r)
            info->port = r;
        else
            info->port = port_path;
    } else
        info->port = "80";
    if (port_path_copy)
        info->path = port_path_copy + strlen(info->port ? info->port : "");
    else {
        char *path = strstr(URL + 8, "/");
        info->path = path ? path : "/";
    }
    int r = strcmp(info->protocol, info->host) == 0;
    if (r && strcmp(info->port, "80") == 0)
        info->protocol = "http";
    else if (r)
        info->protocol = "tcp";
    return info;
}
