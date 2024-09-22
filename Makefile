CC = gcc
CFLAGS = -Wall -pthread -I includes

SERVER_SRC = src/server/server.c src/server/ring_buffer.c src/server/reads_list.c src/server/main.c
CLIENT_SRC = src/client/client.c src/client/main.c src/client/utils.c

SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

SERVER_EXEC = server
CLIENT_EXEC = client

# Default target
all: $(SERVER_EXEC) $(CLIENT_EXEC)

# Build server
$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER_EXEC) $(SERVER_OBJ)

# Build client
$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT_EXEC) $(CLIENT_OBJ)

# Rules for building object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_EXEC) $(CLIENT_EXEC)

# Rebuild everything
rebuild: clean all
