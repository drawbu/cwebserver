#include "server.h"

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void handle_client(int *fd)
{
    char *buffer = malloc(BUFSIZ * sizeof(char));

    if (buffer == NULL) {
        perror("malloc");
        return;
    }
    ssize_t bytes_read = recv(*fd, buffer, BUFSIZ, 0);
    printf("Received %ld bytes\n\n%s", bytes_read, buffer);

    char *response =
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "\n"
        "<!DOCTYPE html> <body>hello world</body>\n";
    size_t response_len = strlen(response);
    send(*fd, response, response_len, 0);
    free(buffer);
    close(*fd);
    free(fd);
}

int main(int argc, char *argv[])
{
    int port = (argc > 1) ? atoi(argv[1]) : 8080;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY,
    };

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("listen");
        return 1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int clientfd =
            accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (clientfd < 0) {
            perror("accept");
            return 1;
        }

        int *fd = malloc(sizeof(int));
        if (fd == NULL) {
            perror("malloc");
            return 1;
        }
        *fd = clientfd;
        pthread_t thread = 0;
        pthread_create(&thread, NULL, (void *(*)(void *))handle_client, fd);
        pthread_detach(thread);
    }
}
