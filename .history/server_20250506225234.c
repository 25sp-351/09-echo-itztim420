#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DEFAULT_PORT 2345
#define MAX_BUFFER 1024

int verbose = 0;

void* handle_connection(void* client_socket) {
    int sock = *((int*)client_socket);
    char buffer[MAX_BUFFER];

    // Read the request
    int bytes_read = read(sock, buffer, MAX_BUFFER);
    if (bytes_read <= 0) {
        close(sock);
        free(client_socket);
        return NULL;
    }

    if (verbose) {
        printf("Received request:\n%s\n", buffer);
    }

    // Create a basic HTTP response
    char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
    
    // Send the HTTP response
    write(sock, response, strlen(response));
    close(sock);
    free(client_socket);
    return NULL;
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        }
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started, listening on port %d\n", port);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connection accepted\n");

        // Create a new thread to handle the client connection
        pthread_t thread_id;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        if (pthread_create(&thread_id, NULL, handle_connection, (void*)client_socket) != 0) {
            perror("Thread creation failed");
            close(new_socket);
        }
    }

    return 0;
}