#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DEFAULT_PORT 2345
#define MAX_BUFFER 1024

int verbose = 0;

// Thread function to handle both HTTP and TCP
void* handle_connection(void* client_socket) {
    int sock = *((int*)client_socket);
    free(client_socket);

    char buffer[MAX_BUFFER];
    ssize_t bytes_read = read(sock, buffer, MAX_BUFFER - 1);

    if (bytes_read <= 0) {
        close(sock);
        return NULL;  // Client closed connection or error
    }

    buffer[bytes_read] = '\0';  // Null-terminate

    // If it's an HTTP request
    if (strncmp(buffer, "GET", 3) == 0) {
        if (verbose) {
            printf("Received HTTP request: %s\n", buffer);
        }

        const char* response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/plain\r\n"
                               "Connection: close\r\n\r\n"
                               "Hello, World!\n";
        write(sock, response, strlen(response));
    } else {
        // Print and echo the first line
        if (verbose) {
            printf("Received TCP message: %s\n", buffer);
        }
        if (write(sock, buffer, bytes_read) < 0) {
            perror("Write failed");
        }

        // Continue echoing
        while ((bytes_read = read(sock, buffer, MAX_BUFFER - 1)) > 0) {
            buffer[bytes_read] = '\0';
            if (verbose) {
                printf("Received TCP message: %s\n", buffer);
            }
            if (write(sock, buffer, bytes_read) < 0) {
                perror("Write failed");
                break;
            }
        }
    }

    close(sock);
    return NULL;
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else {
            fprintf(stderr, "Usage: %s [-p port] [-v]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow address reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    // Accept and handle connections
    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Allocate socket pointer for thread
        int* client_socket = malloc(sizeof(int));
        if (!client_socket) {
            perror("Memory allocation failed");
            close(new_socket);
            continue;
        }
        *client_socket = new_socket;

        // Spawn a new thread
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_connection, client_socket) != 0) {
            perror("Thread creation failed");
            close(new_socket);
            free(client_socket);
            continue;
        }

        pthread_detach(thread_id); // Auto cleanup
    }

    close(server_fd);
    return 0;
}