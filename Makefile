CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
LDFLAGS = -pthread
SRC_DIR = src
TEST_DIR = tests
BIN_DIR = bin
SERVER = $(BIN_DIR)/mini-kv-server
CLIENT = $(BIN_DIR)/mini-kv-cli
TEST = $(BIN_DIR)/test-all
SERVER_SOURCES = $(SRC_DIR)/server.c $(SRC_DIR)/client_handler.c $(SRC_DIR)/commands.c $(SRC_DIR)/hashtable.c $(SRC_DIR)/protocol.c $(SRC_DIR)/persistence.c
CLIENT_SOURCES = $(SRC_DIR)/cli_client.c $(SRC_DIR)/protocol.c
TEST_SOURCES = $(TEST_DIR)/test_all.c $(SRC_DIR)/commands.c $(SRC_DIR)/hashtable.c $(SRC_DIR)/persistence.c
.PHONY: all server client test integration benchmark clean
all: server client test
server: $(SERVER)
client: $(CLIENT)
integration: all
	python3 tests/test_integration.py
benchmark: all
	@echo "Start the server in another terminal, then run: python3 tests/benchmark.py 127.0.0.1 6380 10000"
test: $(TEST)
	$(TEST)
	python3 tests/test_integration.py
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
$(SERVER): $(SERVER_SOURCES) $(SRC_DIR)/commands.h $(SRC_DIR)/hashtable.h $(SRC_DIR)/protocol.h $(SRC_DIR)/persistence.h $(SRC_DIR)/client_handler.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SOURCES) $(LDFLAGS)
$(CLIENT): $(CLIENT_SOURCES) $(SRC_DIR)/protocol.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SOURCES) $(LDFLAGS)
$(TEST): $(TEST_SOURCES) $(SRC_DIR)/commands.h $(SRC_DIR)/hashtable.h $(SRC_DIR)/persistence.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $(TEST_SOURCES) $(LDFLAGS)
clean:
	rm -f $(SERVER) $(CLIENT) $(TEST) mini-kv.aof test-mini-kv.aof test-replay.aof
