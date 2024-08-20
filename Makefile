# i have no idea what im doing
# yes i used chatgpt
# and tutorials from google

CC=gcc
CFLAGS=-Wall -std=c99 -Isrc -Ivendor/raylib/src -DPLATFORM_DESKTOP
LDFLAGS=-lraylib -lm -lpthread -ldl -lrt -lX11

SRC_DIR=src
BUILD_DIR=build

SRC=$(wildcard $(SRC_DIR)/*.c)
OBJ=$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))
BIN=$(BUILD_DIR)/scedit

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/*.o $(BIN)

.PHONY: all clean
