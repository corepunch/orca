CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2

.PHONY: all run clean

all: three-stars

three-stars: main.c world.c world.h
	$(CC) $(CFLAGS) main.c world.c -o $@

run: three-stars
	./three-stars

clean:
	rm -f three-stars
