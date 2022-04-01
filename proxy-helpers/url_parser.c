#include "url_parser.h"


void init_url_info(URL_INFO *info)
{
    info->host = Malloc(MAXLINE);
    info->port = Malloc(MAXLINE);
    info->path = Malloc(MAXLINE);
    info->protocol = Malloc(MAXLINE);
}

void free_url_info(URL_INFO *info)
{
    Free(info->host);
    Free(info->port);
    Free(info->path);
    Free(info->protocol);

    Free(info);
}


/*
 * parse_url - get plain URL and parse it into
 *      [protocol,host,port,path] in a URL_INFO struct
 */
URL_INFO *parse_url(URL_INFO *info, const char *url)
{
    if (!info || !url)
        return NULL;

    char *save_ptr;
    init_url_info(info);

    char *url_copy = (char *)Malloc(strlen(url) + 1);
    char *temp_host = (char *)Malloc(strlen(url) + 1);
    char *temp_host_ptr = temp_host;
    strcpy(url_copy, url);
    strcpy(info->protocol, __strtok_r(url_copy, "://", &save_ptr));
    strcpy(temp_host, strstr(url, "://"));

    if (temp_host) {
        temp_host += 3;
        strcpy(info->host, temp_host);
        char *host_port_path = (char *)Calloc(1, strlen(info->host) + 1);
        strcpy(host_port_path, info->host);
        strcpy(info->host, __strtok_r(host_port_path, ":", &save_ptr));
        strcpy(info->host, __strtok_r(host_port_path, "/", &save_ptr));
        Free(host_port_path);
    } else {
        char *host_port_path = (char *)Calloc(1, strlen(url) + 1);
        strcpy(host_port_path, url);
        strcpy(info->host, __strtok_r(host_port_path, ":", &save_ptr));
        strcpy(info->host, __strtok_r(host_port_path, "/", &save_ptr));
        Free(host_port_path);
    }
    char *URL = (char *)Malloc(strlen(url) + 1);
    strcpy(URL, url);
    char *explicit_port = strstr(URL + 6, ":");
    char *port_path = 0;
    char *port_path_copy = 0;
    if (explicit_port && isdigit(*(port_path = (char *)explicit_port + 1))) {
        port_path_copy = (char *)Malloc(strlen(port_path) + 1);
        strcpy(port_path_copy, port_path);
        char *r = __strtok_r(port_path, "/", &save_ptr);
        if (r)
            strcpy(info->port, r);
        else
            strcpy(info->port, port_path);
    } else {
        strcpy(info->port, "80");
    }
    if (port_path_copy) {
        strcpy(info->path, port_path_copy + strlen(info->port ? info->port : ""));
    }
    else {
        char *path = strstr(URL + 8, "/");
        strcpy(info->path, path ? path : "/");
    }
    int r = strcmp(info->protocol, info->host) == 0;
    if (r && strcmp(info->port, "80") == 0)
        strcpy(info->protocol, "http");
    else if (r)
        strcpy(info->protocol, "tcp");
    
    Free(url_copy);
    Free(temp_host_ptr);
    Free(URL);
    Free(port_path_copy);
    return info;
}
