TARGETNAME = orca.vm
DIR_BUILD = ../build
DIR_OBJ = $(DIR_BUILD)/obj
DIR_BIN = $(DIR_BUILD)/bin
TARGET = $(DIR_BIN)/$(TARGETNAME)
MAKE = make
CC = gcc
ASM = /Users/igor/Developer/armvm/Build/Products/Debug/armvm
CFLAGS = -O0 -DLUA_32BITS -DCOMPILE_VM -S -fverbose-asm -fno-stack-protector -D_FORTIFY_SOURCE=0 -arch arm -g2 -fpic -I..
MKDIRP = mkdir -p
INCOMING = . scene ui ../lua54 ../cluster ../dataplugin ../shared
#INCOMING = binding math object tween unix renderer scene sprite ui cluster filesystem network common system
DIRECTORIES = $(DIR_OBJ) $(DIR_BIN)
SOURCES = $(foreach dir, $(INCOMING), $(wildcard $(dir)/*.c))
OBJECTS = $(patsubst %.c, $(DIR_OBJ)/%.s, $(SOURCES))

.PHONY: default all clean directories dependencies

default: directories dependencies $(TARGET)
all: default
build: default
directories:
	$(patsubst %, $(MKDIRP) %;, $(addprefix $(DIR_OBJ)/,$(INCOMING)))
	$(patsubst %, $(MKDIRP) %;, $(DIRECTORIES))

$(DIR_OBJ)/%.s: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(ASM) syscalls.asm $(OBJECTS) $(patsubst %, -l%, $(LIBS)) -Wall -o $@ $(LDFLAGS)

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)

run:
	$(TARGET) --data=../projects/common --data=../projects/cluster --data=../amg-full --config=../projects/cluster.xproj