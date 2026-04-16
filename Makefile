BIN_DIR=./build/bin
OBJ_DIR=./build/obj
SRC_DIR=./src
EXAMPLE_DIR=./examples

CC=clang
INCLUDES=-Iinclude
CFLAGS=-g -Wall -Wextra $(INCLUDES)
LDFLAGS=-v

# Output files
SHARED = $(BIN_DIR)/libcu132.so
ARCHIVE = $(BIN_DIR)/libcu132.a
MONITOR = $(BIN_DIR)/monitor

all: library examples

library: $(SHARED) $(ARCHIVE)

examples: $(MONITOR)

# Rule to build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to build the library (shared object)
$(SHARED): $(OBJ_DIR)/libcu132.o
	mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -shared $< -lserialport -o $@

# Rule to build the library (archive)
$(ARCHIVE): $(OBJ_DIR)/libcu132.o
	mkdir -p $(BIN_DIR)
	ar rcs $@ $<

# Rule to build the example binary (monitor)
$(MONITOR): $(EXAMPLE_DIR)/monitor.c $(ARCHIVE)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $< $(ARCHIVE) -lserialport -o $@

clean:
	rm $(BIN_DIR)/*
	rm $(OBJ_DIR)/*

.PHONY: all library examples clean