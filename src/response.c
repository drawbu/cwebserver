#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common_types.h"
#include "debug.h"
#include "server.h"

struct response_s
{
    char *protocol;
    char *code;
    char *message;
    DEF_ARR(header_t) headers;
};

static char *file_not_found(size_t *size)
{
    static const char err[] = "404 Not Found\r\n";
    *size = sizeof err - 1;
    return strdup(err);
}

static char *open_file(size_t *size, const char *path, struct response_s *res)
{
    char *buffer = NULL;

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        DEBUG("Error: open %s", strerror(errno));
        buffer = file_not_found(size);
    } else {
        buffer = malloc(BUFSIZ * sizeof(char));
        if (buffer == NULL)
            return perror("malloc"), NULL;
        DEBUG("file fd: %d", fd);
        ssize_t r_size = read(fd, buffer, BUFSIZ);
        if (r_size != -1) {
            *size = r_size;
            buffer[*size] = '\0';
        } else {
            DEBUG("Error: open %s", strerror(errno));
            DEBUG("Trying %sindex.html", path);
            strcpy(buffer, path);
            if (buffer[strlen(buffer) - 1] != '/')
                strcat(buffer, "/");
            strcat(buffer, "index.html");
            char *e = open_file(size, buffer, res);
            free(buffer);
            return e;
        }
        close(fd);
    }

    // Content-Length: <size>
    header_t content_lenght = {
        .key = strdup("Content-Length"),
        .value = malloc(32 * sizeof(char)),
    };
    size_t w = sprintf(content_lenght.value, "%ld", *size);
    content_lenght.value[w] = '\0';

    append_to_array(&res->headers, &content_lenght, sizeof(header_t));

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

static int compare(const char *needle, const struct mime_type_s *item)
{
    return strcmp(needle, item->ext);
}

static void init_headers(request_t *args, struct response_s *res)
{
    res->protocol = "HTTP/1.1";
    res->code = "200";
    res->message = "OK";

    char *ext = strrchr(args->path, '.');
    const char *type = "text/html";
    if (ext != NULL && *ext != '\0') {
        struct mime_type_s *item = bsearch(
            ext + 1, COMMON_TYPES, LENOF(COMMON_TYPES), sizeof(COMMON_TYPES[0]),
            (int (*)(const void *, const void *))compare);
        if (item != NULL)
            type = item->mime;
    }

    append_to_array(
        &res->headers,
        &(header_t){
            .key = strdup("Content-Type"),
            .value = strdup(type),
        },
        sizeof(header_t));
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
    init_headers(args, &res);

    size_t filesize = 0;
    char *filebuf = open_file(&filesize, path, &res);

    size_t send_size = 0;
    char *send_buf = create_response(filebuf, filesize, &res, &send_size);
    write(args->fd, send_buf, send_size);
    DEBUG_DO(write(STDOUT_FILENO, send_buf, send_size));

    free(filebuf);
    free(send_buf);
    for (size_t i = 0; i < res.headers.size; i++) {
        free(res.headers.arr[i].key);
        free(res.headers.arr[i].value);
    }
    free(path);
    return 0;
}
