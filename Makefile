CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
LDFLAGS = -pthread

SRC_DIR = src
BIN_DIR = bin

SERVER_SOURCES = \
	$(SRC_DIR)/server.c \
	$(SRC_DIR)/client_handler.c \
	$(SRC_DIR)/hashtable.c \
	$(SRC_DIR)/commands.c \
	$(SRC_DIR)/protocol.c \
	$(SRC_DIR)/persistence.c

CLIENT_SOURCES = \
	$(SRC_DIR)/cli_client.c \
	$(SRC_DIR)/protocol.c

SERVER = $(BIN_DIR)/mini-kv-server
CLIENT = $(BIN_DIR)/mini-kv-cli

.PHONY: all clean

all: $(SERVER) $(CLIENT)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(SERVER): $(SERVER_SOURCES) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SOURCES) $(LDFLAGS)

$(CLIENT): $(CLIENT_SOURCES) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SOURCES)

clean:
	rm -f $(SERVER) $(CLIENT)