#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    int verbose = 0;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        }
    }

    start_server(port, verbose);
    return 0;
}