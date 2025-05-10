#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 2345
#define BUFFER_SIZE 1024

int verbose = 0;

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer)-1)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate
        if (verbose) {
            printf("Received: %s", buffer);
        }
        write(client_socket, buffer, bytes_read);  // Echo back
    }

    close(client_socket);
    if (verbose) {
        printf("Client disconnected.\n");
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            verbose = 1;
        } else if (!strcmp(argv[i], "-p") && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Usage: %s [-v] [-p port]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (*client_socket < 0) {
            perror("accept");
            free(client_socket);
            continue;
        }

        if (verbose) {
            printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_socket);
        pthread_detach(tid); // Auto-cleanup
    }

    close(server_socket);
    return 0;
}