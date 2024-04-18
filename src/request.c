#include "server.h"

#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void parse_request(request_t *req, char *buffer)
{
    if (req == NULL || buffer == NULL)
        return;
    printf("Parsing request\n");
    char *ptr = strsep(&buffer, "\n");
    char *method = strsep(&ptr, " ");;
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
        printf("key=%s, value=%s\n", header.key, header.value);
        append_to_array(&req->headers, &header, sizeof header);
    }
}

void handle_client(request_t *args)
{
    char *buffer = malloc(BUFSIZ * sizeof(char));

    if (buffer == NULL) {
        perror("malloc");
        return;
    }
    read(args->fd, buffer, BUFSIZ);
    /* printf("Received %ld bytes\n\n%s", bytes_read, buffer); */

    parse_request(args, buffer);
    printf("Method: %s\n", request_type_str[args->method]);
    printf("Path: %s\n", args->path);

    char *response =
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "\n"
        "<!DOCTYPE html> <body>hello world</body>\n";
    size_t response_len = strlen(response);
    send(args->fd, response, response_len, 0);
    free(buffer);
    close(args->fd);
    free(args);
}

