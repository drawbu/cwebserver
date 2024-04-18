#include "server.h"

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void *append_to_array(void *array, void *elem, size_t size)
{
    DEF_ARR(uint8_t) *arr = array;

    if (arr->size + 1 > arr->alloc) {
        arr->alloc = (arr->alloc) ? arr->alloc * arr->alloc : 2;
        arr->arr = reallocarray(arr->arr, arr->alloc + 1, size);
        if (arr->arr == NULL)
            return NULL;
    }
    memcpy(arr->arr + (arr->size * size), elem, size);
    arr->size += 1;
    return array;
}

int main(int argc, char *argv[])
{
    server_t serv = {0};

    if (argc != 3) {
        fprintf(stderr, "Invalid number of arguments\n");
        return 1;
    }

    serv.port = (short)strtol(argv[1], NULL, 10);
    realpath(argv[2], serv.root);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) ==
        -1)
        return 1;

    serv.addr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_port = htons(serv.port),
        .sin_addr.s_addr = INADDR_ANY,
    };

    if (bind(sockfd, (struct sockaddr *)&serv.addr, sizeof(serv.addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server running on http://localhost:%d\n", serv.port);
    printf("Serving files from %s\n", serv.root);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int clientfd =
            accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (clientfd < 0) {
            perror("accept");
            return 1;
        }

        request_t *req = malloc(sizeof *req);
        if (req == NULL) {
            perror("malloc");
            return 1;
        }
        memset(req, 0, sizeof *req);
        req->fd = clientfd;
        req->server = &serv;
        pthread_t thread = 0;
        pthread_create(&thread, NULL, (void *(*)(void *))handle_client, req);
        pthread_detach(thread);
    }
}
