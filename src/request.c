#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "debug.h"
#include "server.h"

static void parse_request(request_t *req, char *buffer)
{
    if (req == NULL || buffer == NULL)
        return;
    DEBUG_MSG("Parsing request");
    char *ptr = strsep(&buffer, "\n");
    char *method = strsep(&ptr, " ");
    ;
    char *path = strsep(&ptr, " ");
    if (method == NULL || path == NULL)
        return;
    for (size_t i = 0; i < LENOF(request_type_str); i++) {
        if (strcmp(method, request_type_str[i]) == 0) {
            req->method = i;
            break;
        }
    }
    req->path = path;

    while (1) {
        if (buffer == NULL)
            break;
        header_t header = {0};
        char *line = strsep(&buffer, "\n");
        if (line == NULL)
            break;
        char *value = line;
        char *key = strsep(&value, ":");
        if (key == NULL || value == NULL)
            break;
        header.key = key;
        while (isspace(*value))
            value++;
        header.value = value;
        DEBUG("key=%s, value=%s", header.key, header.value);
        append_to_array(&req->headers, &header, sizeof header);
    }
}

void *handle_client(request_t *args)
{
    char *buffer = malloc(BUFSIZ * sizeof(char));

    if (buffer == NULL) {
        perror("malloc");
        return NULL;
    }

    read(args->fd, buffer, BUFSIZ);

    parse_request(args, buffer);
    DEBUG("Method: %s", request_type_str[args->method]);
    DEBUG("Path: %s", args->path);

    response_to_client(args);

    free(buffer);
    close(args->fd);
    free(args);
    return NULL;
}
