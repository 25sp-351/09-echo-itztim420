#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DEFAULT_PORT 2345

// Function to handle each client connection
void *handle_connection(void *client_socket) {
    int sock = *(int *)client_socket;
    char buffer[1024];
    int read_size;

    // Read data from the client (request)
    while ((read_size = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0';  // Null-terminate the string

        // Check if verbose flag is set, and if so, print the request
        printf("Received: %s\n", buffer);

        // Send an HTTP response back to the client (basic echo response)
        char response[2048];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Connection: close\r\n"
                 "\r\n"
                 "%s", buffer);

        send(sock, response, strlen(response), 0);
    }

    // Clean up the client socket
    close(sock);
    free(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket, *new_sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);
    pthread_t thread;

    // Set the default port to 2345
    int port = DEFAULT_PORT;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[i + 1]);
            } else {
                fprintf(stderr, "Error: Port number missing after '-p'\n");
                return 1;
            }
        }
    }

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 1) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    // Accept incoming connections and handle them with threads
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len))) {
        printf("Connection accepted\n");

        // Create a new thread to handle the connection
        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        if (pthread_create(&thread, NULL, handle_connection, (void *)new_sock) < 0) {
            perror("Thread creation failed");
            return 1;
        }

        // Detach the thread to allow it to clean up after itself
        pthread_detach(thread);
    }

    if (client_socket < 0) {
        perror("Accept failed");
        return 1;
    }

    return 0;
}