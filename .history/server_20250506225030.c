#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>

#define MAX_BUFFER 1024

// Function to handle each client connection
void *handle_connection(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[MAX_BUFFER];

    // Read the request from the client
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("Error reading from client");
        close(client_socket);
        return NULL;
    }

    // Null terminate the buffer to make it a proper C string
    buffer[bytes_read] = '\0';

    // Check if the request is an HTTP GET request (basic check)
    if (strncmp(buffer, "GET", 3) == 0) {
        // Prepare the HTTP response (echo the received message)
        char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
        write(client_socket, response_header, strlen(response_header));
        write(client_socket, buffer, bytes_read);
    } else {
        // Send an error response if the request is not GET
        char *error_response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid HTTP Request";
        write(client_socket, error_response, strlen(error_response));
    }

    // Close the client socket
    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = 2345;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        return -1;
    }

    printf("Server listening on port %d...\n", port);

    // Accept connections and spawn a thread to handle each one
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connection established with client\n");

        // Create a new thread to handle the connection
        pthread_create(&thread_id, NULL, handle_connection, (void *)&client_socket);
        pthread_detach(thread_id);  // Detach so the thread will clean up itself
    }

    close(server_socket);
    return 0;
}