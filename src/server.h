#pragma once

#include <linux/limits.h>
#include <netinet/in.h>

#include "utils.h"

enum request_method_e {
    GET,
    POST,
    PUT,
    DELETE,
};

static const char *const request_type_str[] = {
    [GET] = "GET",
    [POST] = "POST",
    [PUT] = "PUT",
    [DELETE] = "DELETE",
};

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
    enum request_method_e method;
    char *path;
} request_t;

void *handle_client(request_t *args);
int response_to_client(request_t *args);
