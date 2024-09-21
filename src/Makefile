# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -pthread -I$(INC_DIR)

# Directories
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin

# Source files
CLIENT_SRC = $(SRC_DIR)/client.c
UTILS_SRC = $(SRC_DIR)/utils.c  # Add any additional source files here

# Object files
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
UTILS_OBJ = $(UTILS_SRC:.c=.o)

# Executables
CLIENT_EXEC = $(BIN_DIR)/client

# Default target
all: $(CLIENT_EXEC)

# Compile client
$(CLIENT_EXEC): $(CLIENT_OBJ) $(UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(CLIENT_OBJ) $(UTILS_OBJ) $(CLIENT_EXEC)

.PHONY: all clean
