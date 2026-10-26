PLUGINNAME = level
SDKDIR = ../sdk
INCLUDEDIR = ../..
BUILDDIR = build
TARGET = ../lib$(PLUGINNAME).so
LIBS = -lm -llua
CC = gcc
CFLAGS = -g -Wall -fpic -I$(INCLUDEDIR) -I/opt/homebrew/include
LDFLAGS = -shared -L/opt/homebrew/lib

.PHONY: default all CLEAN

default: directories $(TARGET)
all: default
directories: ${BUILDDIR}

OBJECTS = $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

$(BUILDDIR)/%.o: %.c $(HEADERS)
		$(CC) $(CFLAGS) -c $< -o $@

${BUILDDIR}:
		${MKDIRP} ${BUILDDIR}

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
		$(CC) $(OBJECTS) -Wall $(LIBS) -o $@ $(LDFLAGS)

clean:
		-rm -f $(BUILDDIR)/*.o
		-rm -f $(TARGET)