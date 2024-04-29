#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "debug.h"
#include "server.h"

struct response_s
{
    char *protocol;
    char *code;
    char *message;
    DEF_ARR(header_t) headers;
};

static char *open_file(size_t *size, const char *path, struct response_s *res)
{
    static const char err[] = "404 Not Found\n";
    char *buffer = malloc(BUFSIZ * sizeof(char));

    if (buffer == NULL)
        return perror("malloc"), NULL;

    strcpy(buffer, err);
    *size = sizeof err;

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        DEBUG("Error: open %s", strerror(errno));
    } else {
        DEBUG("file fd: %d", fd);
        *size = read(fd, buffer, BUFSIZ);
        buffer[*size] = '\0';
        close(fd);
    }

    // Content-Length: <size>
    header_t content_lenght = {
        .key = strdup("Content-Length"),
        .value = malloc(32 * sizeof(char)),
    };
    size_t w = sprintf(content_lenght.value, "%ld", *size);
    content_lenght.value[w] = '\0';

    append_to_array(
        &res->headers,
        &content_lenght,
        sizeof(header_t));

    return buffer;
}

static char *create_response(
    const char *filebuf, size_t filesize, struct response_s *res, size_t *size)
{
    char_array_t buf = {0};

    append_to_buffer(&buf, res->protocol, strlen(res->protocol));
    append_to_buffer(&buf, " ", 1);
    append_to_buffer(&buf, res->code, strlen(res->code));
    append_to_buffer(&buf, " ", 1);
    append_to_buffer(&buf, res->message, strlen(res->message));
    append_to_buffer(&buf, "\r\n", 2);

    for (size_t i = 0; i < res->headers.size; i++) {
        append_to_buffer(
            &buf, res->headers.arr[i].key, strlen(res->headers.arr[i].key));
        append_to_buffer(&buf, ": ", 2);
        append_to_buffer(
            &buf, res->headers.arr[i].value, strlen(res->headers.arr[i].value));
        append_to_buffer(&buf, "\r\n", 2);
    }

    append_to_buffer(&buf, "\r\n", 2);
    append_to_buffer(&buf, filebuf, filesize);
    *size = buf.size;
    return buf.arr;
}

int response_to_client(request_t *args)
{
    char *path = malloc(PATH_MAX * sizeof(char));
    if (path == NULL) {
        perror("malloc");
        return 1;
    }
    strcpy(path, args->server->root);
    strcat(path, args->path);

    struct response_s res = {0};
    res.protocol = "HTTP/1.1";
    res.code = "200";
    res.message = "OK";

    append_to_array(
        &res.headers,
        &(header_t){
            .key = strdup("Content-Type"),
            .value = strdup("text/html"),
        },
        sizeof(header_t));

    size_t filesize = 0;
    char *filebuf = open_file(&filesize, path, &res);
    DEBUG("res: %s", filebuf);

    size_t send_size = 0;
    char *send_buf = create_response(filebuf, filesize, &res, &send_size);
    write(args->fd, send_buf, send_size);

    free(filebuf);
    free(send_buf);
    for (size_t i = 0; i < res.headers.size; i++) {
        free(res.headers.arr[i].key);
        free(res.headers.arr[i].value);
    }
    free(path);
    return 0;
}
