CC = gcc
CFLAGS = -Wall -Wextra
SRC_DIR = src
BIN_DIR = bin

all: server client

server: $(BIN_DIR)/server

client: $(BIN_DIR)/client

$(BIN_DIR)/server: $(SRC_DIR)/server.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/client: $(SRC_DIR)/client.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BIN_DIR)/*

.PHONY: all server client clean