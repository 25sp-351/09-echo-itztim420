#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

// Global variable for verbosity control
int verbose = 0;

// Function to handle connections
void *handle_connection(void *client_socket) {
    int sock = *(int*)client_socket;
    char buffer[1024];

    if (verbose) {
        printf("Client connected\n");
    }

    while (1) {
        int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            break; // Connection closed or error
        }
        buffer[n] = '\0';
        if (verbose) {
            printf("Received: %s\n", buffer);
        }
        send(sock, buffer, n, 0);  // Echo back
    }

    if (verbose) {
        printf("Connection closed\n");
    }

    close(sock);
    free(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    // Optional: Parsing command-line arguments for -v and -p (port)
    int port = 2345;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            i++; // Skip next argument (port number)
        }
    }

    int server_socket, client_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Server started on port %d\n", port);

    // Accept incoming connections and create threads for each connection
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Create thread to handle the connection
        pthread_t thread;
        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;
        if (pthread_create(&thread, NULL, handle_connection, (void*)new_sock) < 0) {
            perror("Thread creation failed");
            continue;
        }

        pthread_detach(thread);  // Allow thread to clean up after itself
    }

    close(server_socket);
    return 0;
}