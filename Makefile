# i have no idea what im doing
# yes i used chatgpt
# and tutorials from google

CC=gcc
CFLAGS=-Wall -std=c99 -Isrc -Ivendor/raylib/src -DPLATFORM_DESKTOP
LDFLAGS=-lraylib -lm -lpthread -ldl -lrt -lX11

SRC=src/main.c
BIN=build/sceditor

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

clean:
	rm -f $(BIN)

.PHONY: all clean
