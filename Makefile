BIN_DIR=./build/bin
OBJ_DIR=./build/obj
SRC_DIR=./src

CC=clang
INCLUDES=-Iexternal/libserialport -Iinclude
LIBS=-Lexternal/libserialport/.libs -L$(BIN_DIR)
CFLAGS=-g -Wall -Wextra $(INCLUDES)
LDFLAGS=$(LIBS)

# Output files
SHARED = $(BIN_DIR)/libcu132.so
ARCHIVE = $(BIN_DIR)/libcu132.a
BINARY = $(BIN_DIR)/cu-monitor

all: library binary

library: $(SHARED) $(ARCHIVE)

binary: $(BINARY)

# Rule to build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to build the library (shared object)
$(SHARED): $(OBJ_DIR)/libcu132.o
	mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -shared -lserialport -o $@ $<

# Rule to build the library (archive)
$(ARCHIVE): $(OBJ_DIR)/libcu132.o
	mkdir -p $(BIN_DIR)
	ar rcs $@ $<

# Rule to build the binary (monitor)
$(BINARY): $(OBJ_DIR)/monitor.o $(ARCHIVE)
	mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ -lserialport $< $(ARCHIVE)

clean:
	rm ./$(BIN_DIR)/*
	rm ./$(OBJ_DIR)/*

.PHONY: all library binary clean