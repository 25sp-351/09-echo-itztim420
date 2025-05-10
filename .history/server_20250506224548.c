#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DEFAULT_PORT 2345
#define MAX_LINE 1024

void *handle_connection(void *client_sock_ptr) {
    int client_sock = *(int *)client_sock_ptr;
    free(client_sock_ptr);  // Free the memory allocated for the client socket pointer

    char buffer[MAX_LINE];
    ssize_t bytes_read;

    // Read the HTTP request from the client
    bytes_read = read(client_sock, buffer, sizeof(buffer)-1);
    if (bytes_read <= 0) {
        close(client_sock);
        return NULL;
    }

    // Null-terminate the received data
    buffer[bytes_read] = '\0';

    // Send the HTTP response (echo the request back)
    const char *http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
    write(client_sock, http_response, strlen(http_response));  // HTTP header
    write(client_sock, buffer, bytes_read);  // Echo back the request content

    close(client_sock);  // Close the client connection
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    int verbose = 0;
    int opt;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':  // Set port
                port = atoi(optarg);
                break;
            case 'v':  // Verbose mode
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Create the socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket to the address
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sock, 1) == -1) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        // Accept a new connection
        int *client_sock = malloc(sizeof(int));  // Allocate memory for the client socket
        *client_sock = accept(server_sock, NULL, NULL);
        if (*client_sock == -1) {
            perror("Accept failed");
            free(client_sock);  // Free memory in case of error
            continue;
        }

        // Print verbose information about the new connection
        printf("New connection accepted...\n");

        // Create a new thread to handle the connection
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_connection, client_sock) != 0) {
            perror("Thread creation failed");
            close(*client_sock);  // Close the client socket if thread creation fails
            free(client_sock);  // Free the allocated memory
        } else {
            pthread_detach(thread_id);  // Detach the thread so that it cleans up after itself
        }
    }

    close(server_sock);  // Close the server socket (unreachable code)
    return 0;
}