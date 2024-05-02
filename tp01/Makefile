CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99
CLIENT_SRC=client.c
SERVER_SRC=server.c
CLIENT_OBJ=$(CLIENT_SRC:.c=.o)
SERVER_OBJ=$(SERVER_SRC:.c=.o)
BIN_DIR=bin
CLIENT_BIN=$(BIN_DIR)/client
SERVER_BIN=$(BIN_DIR)/server

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(SERVER_BIN): $(SERVER_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< -lm

$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SERVER_OBJ): $(SERVER_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(CLIENT_BIN) $(SERVER_BIN)

.PHONY: all clean
    