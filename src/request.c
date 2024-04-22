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

static bool file_exists(const char *filename)
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0) && S_ISREG(buffer.st_mode);
}

static void open_file(char *buffer, size_t *size, const char *path)
{
    char *filebuf = malloc(BUFSIZ * sizeof(char));
    size_t filesize = 0;

    if (filebuf == NULL) {
        perror("malloc");
        return;
    }
    if (!file_exists(path)) {
        DEBUG_MSG("File not found");
        static const char err[] = "404 Not Found\n";
        strcpy(filebuf, err);
        filesize = sizeof err;
    } else {
        int fd = open(path, O_RDONLY);
        DEBUG("file fd: %d", fd);
        filesize = read(fd, filebuf, BUFSIZ);
        close(fd);
    }
    *size += sprintf(buffer + *size, "Content-Length: %ld\n\n", filesize);
    memcpy(buffer + *size, filebuf, filesize);
    *size += filesize;
    free(filebuf);
}

void handle_client(request_t *args)
{
    char *buffer = malloc(BUFSIZ * sizeof(char));

    if (buffer == NULL) {
        perror("malloc");
        return;
    }

    read(args->fd, buffer, BUFSIZ);

    parse_request(args, buffer);
    DEBUG("Method: %s", request_type_str[args->method]);
    DEBUG("Path: %s", args->path);
    char *path = malloc(PATH_MAX * sizeof(char));
    if (path == NULL) {
        perror("malloc");
        return;
    }
    strcpy(path, args->server->root);
    strcat(path, args->path);

    static const char response[] =
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n";
    char *send_buf = malloc(BUFSIZ * sizeof(char));
    if (send_buf == NULL) {
        free(path);
        perror("malloc");
        return;
    }
    strcpy(send_buf, response);
    size_t size = sizeof response - 1;
    open_file(send_buf, &size, path);

    DEBUG("res: %s", send_buf);
    write(args->fd, send_buf, size);
    free(path);
    free(buffer);
    free(send_buf);
    close(args->fd);
    free(args);
}
