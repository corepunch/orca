CC      := gcc
CFLAGS  := -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -O2 $(shell pkg-config --cflags libxml-2.0)
LDFLAGS := $(shell pkg-config --libs libxml-2.0)

SRCS := main.c model.c tmpl.c
OBJS := $(SRCS:.c=.o)
BIN  := codegen

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c model.h tmpl.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(BIN)
