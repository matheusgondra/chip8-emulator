CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c23 -Iinclude
SRCS = src/main.c src/cpu.c

OBJS = $(SRCS:src/%.c=build/%.o)
TARGET = build/chip8

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build