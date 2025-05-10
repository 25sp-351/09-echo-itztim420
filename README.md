# TCP Echo Server

This is a multi-threaded TCP echo server written in C. It echoes each line sent by a client and can handle basic HTTP GET requests. It supports two command-line options: `-p` to specify the port (default is 2345), and `-v` to enable verbose mode which prints received messages to the terminal.

To compile the server, run `make`. This will generate the `server` executable. To start the server on port 2345 with verbose mode, use `./server -p 2345 -v`. You can test the server using Telnet with `telnet localhost 2345`, then type messages and press Enter to see them echoed back. You can also test HTTP handling by visiting `http://localhost:2345/something` in your browser, and the server will respond with `Hello, World!`.

To clean up the compiled binary, run `make clean`.