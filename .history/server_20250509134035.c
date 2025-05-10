#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void* handle_connection(void* arg) {
    client_args_t* args = (client_args_t*)arg;
    int sock = args->client_socket;
    int verbose = args->verbose;
    char buffer[MAX_BUFFER];

    while (1) {
        ssize_t len = read(sock, buffer, MAX_BUFFER - 1);
        if (len <= 0) break;

        buffer[len] = '\0';
        if (verbose) {
            printf("Received: %s", buffer);
        }

        // Echo back the message
        if (write(sock, buffer, len) <= 0) break;
    }

    close(sock);
    free(args);
    return NULL;
}

void start_server(int port, int verbose) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New connection accepted\n");

        pthread_t thread_id;
        client_args_t* args = malloc(sizeof(client_args_t));
        args->client_socket = new_socket;
        args->verbose = verbose;

        if (pthread_create(&thread_id, NULL, handle_connection, args) != 0) {
            perror("Thread creation failed");
            close(new_socket);
            free(args);
        } else {
            pthread_detach(thread_id); // Prevent memory leak from joinable threads
        }
    }

    close(server_fd);
}