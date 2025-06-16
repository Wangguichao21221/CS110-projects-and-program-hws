CC = gcc
CFLAGS = -g -std=c11 -Wpedantic -Wall -Wextra -Werror

SRCS = src/tables.c src/utils.c src/translate_utils.c src/translate.c src/block.c assembler.c
OBJS = $(SRCS:.c=.o)

TEST_NAME ?= Pseudo

.PHONY: all clean check test

all: assembler

assembler: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-$(MAKE) -C test clean
	-rm -f *.o src/*.o assembler
	-rm -rf __pycache__

check: assembler
	$(MAKE) -C test check

test: assembler
	$(MAKE) -C test test TEST_NAME=$(TEST_NAME)