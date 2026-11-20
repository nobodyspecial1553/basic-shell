CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic -std=c89 -Wno-long-long -Wno-unused-function

SRC_FILES = $(wildcard *.c)
OBJ_FILES = $(SRC_FILES:.c=.o)

all: basic-shell

basic-shell: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^
	rm $(OBJ_FILES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f basic-shell *.o
