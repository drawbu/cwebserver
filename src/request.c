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

#include "server.h"

static void parse_request(request_t *req, char *buffer)
{
    if (req == NULL || buffer == NULL)
        return;
    printf("Parsing request\n");
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
        printf("key=%s, value=%s\n", header.key, header.value);
        append_to_array(&req->headers, &header, sizeof header);
    }
}

static bool file_exists(char *filename)
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0) && S_ISREG(buffer.st_mode);
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
    printf("Method: %s\n", request_type_str[args->method]);
    printf("Path: %s\n", args->path);
    char *path = malloc(PATH_MAX * sizeof(char));
    if (path == NULL) {
        perror("malloc");
        return;
    }
    strcpy(path, args->server->root);
    strcat(path, args->path);

    static const char response[] =
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "Content-Length: ";
    char *send_buf = malloc(BUFSIZ * sizeof(char));
    if (send_buf == NULL) {
        free(path);
        perror("malloc");
        return;
    }

    char *filebuf = malloc(BUFSIZ * sizeof(char));
    if (filebuf == NULL) {
        free(path);
        free(send_buf);
        perror("malloc");
        return;
    }
    size_t filesize = 0;
    size_t size = sizeof response - 1;
    strcpy(send_buf, response);
    printf("path bobibob\n");
    if (!file_exists(path)) {
        printf("File not found\n");
        static const char err[] = "404 Not Found\n";
        strcpy(filebuf, err);
        filesize = sizeof err;
    } else {
        int fd = open(path, O_RDONLY);
        printf("file fd: %d\n", fd);
        filesize = read(fd, filebuf, BUFSIZ);
        close(fd);
    }
    size += sprintf(send_buf + size, "%ld\n\n", filesize);
    memcpy(send_buf + size, filebuf, filesize);
    size += filesize;
    printf("res: %s\n", send_buf);
    write(args->fd, send_buf, size);
    free(path);
    free(buffer);
    free(send_buf);
    free(filebuf);
    close(args->fd);
    free(args);
}
