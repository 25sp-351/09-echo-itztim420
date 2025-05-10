# Compiler and flags
CC = gcc
CFLAGS = -Wall -pthread

# Target executable
TARGET = server

# Default rule
all: $(TARGET)

# Build server from server.c
$(TARGET): server.c
	$(CC) $(CFLAGS) -o $(TARGET) server.c

# Clean up generated files
clean:
	rm -f $(TARGET)