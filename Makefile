BIN_DIR=./build/bin
OBJ_DIR=./build/obj
SRC_DIR=./src

CC=clang
INCLUDES=-I/home/jonathan/Downloads/libserialport-0.1.2 -Iinclude
LIBS=-L/home/jonathan/Downloads/libserialport-0.1.2/.libs -L$(BIN_DIR)
CFLAGS=-g -Wall -Wextra $(INCLUDES)
LDFLAGS=$(LIBS) -static

# Source files
LIB_SRC = $(SRC_DIR)/libcu132.c
TEST_SRC = $(SRC_DIR)/tests/test.c
OBJ_LIB = $(OBJ_DIR)/libcu132.o
OBJ_TEST = $(OBJ_DIR)/test.o

# Output files
LIBRARY = $(BIN_DIR)/libcu132.a
TEST_BIN = $(BIN_DIR)/test

CLANGD_FLAGS = compile_flags.txt

all: $(LIBRARY) $(CLANGD_FLAGS)

# Rule to generate compile_flags.txt
$(CLANGD_FLAGS): 
	echo $(INCLUDES) | tr ' ' '\n' > $(CLANGD_FLAGS)

# Rule to build the library (static archive)
$(LIBRARY): $(OBJ_LIB)
	mkdir -p $(BIN_DIR)
	ar rcs $@ $^

# Rule to build libcu132.o
$(OBJ_LIB): $(LIB_SRC)
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

tests: $(TEST_BIN)

$(TEST_BIN): $(OBJ_TEST) $(LIBRARY)
	mkdir -p $(OBJ_DIR)
	$(CC) $(LDFLAGS) $(OBJ_TEST) -lcu132 -lserialport -o $@

$(OBJ_TEST): $(TEST_SRC)
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm ./$(BIN_DIR)/*
	rm ./$(OBJ_DIR)/*

.PHONY: all clean clean-obj clean-bin tests