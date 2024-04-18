#pragma once

#include <linux/limits.h>
#include <netinet/in.h>

#define DEF_ARR(type) struct { size_t size, alloc; type *arr; }

typedef struct {
    short port;
    char root[PATH_MAX];
    struct sockaddr_in addr;
} server_t;

typedef struct {
    char *key;
    char *value;
} header_t;

typedef struct {
    int fd;
    server_t *server;
    DEF_ARR(header_t) headers;
} request_t;
