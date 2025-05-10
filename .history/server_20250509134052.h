#ifndef SERVER_H
#define SERVER_H

#define DEFAULT_PORT 2345
#define MAX_BUFFER 1024

typedef struct {
    int client_socket;
    int verbose;
} client_args_t;

void start_server(int port, int verbose);

#endif