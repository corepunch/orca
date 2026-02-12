APPNAME = orca
LIBNAME = orca
BUILDDIR = build
SOURCEDIR = source
DATADIR = data
OBJECTDIR = $(BUILDDIR)/obj
SHAREDIR = $(BUILDDIR)/share/$(APPNAME)
BINDIR = $(BUILDDIR)/bin
LIBDIR = $(BUILDDIR)/lib
TARGET = $(BINDIR)/$(APPNAME)
TARGETLIB = $(LIBDIR)/lib$(LIBNAME).so
PLATFORM_LIBDIR = libs/platform
#LIBS = -lm -ldl -lpthread -llua5.4 -lfreetype -lpng -ljpeg -lz -llz4 -lcurl -lxml2 -lplatform
LIBS = -ldl -lpthread -lcurl -lplatform
CC = gcc
# Allow CFLAGS to be passed from luarocks, but ensure we have base flags
CFLAGS ?= -O2 -g
# Always add these flags, even if CFLAGS is passed from outside
CFLAGS += -fpic -I. -I$(CURDIR)
LDFLAGS = -L$(LIBDIR)
MODULES = geometry orca platform sysutil console localization parsers UIKit debug network renderer filesystem core SceneKit vsomeip server editor backend
SOURCEMODULES = $(addprefix ${SOURCEDIR}/, $(MODULES))
OBJECTS = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(wildcard $(dir)/*.c)))
HEADERS = $(wildcard *.h)
SOURCEMODULES2 = $(addprefix /, $(MODULES))
UNITEOBJECTS = $(addsuffix .o, $(MODULES))
UNITE = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(wildcard $(dir)/*.c)))
CFLAGS += $(shell pkg-config --cflags zlib liblz4 lua5.4 libjpeg freetype2 libxml-2.0 2>/dev/null)
LDFLAGS += $(shell pkg-config --libs zlib liblz4 lua5.4 freetype2 libjpeg libpng libxml-2.0 2>/dev/null)

ifeq ($(shell uname -s),Darwin)
	LIBS += -framework OpenGL -framework IOSurface
	LDFLAGS += -Wl,-rpath,@executable_path/../../$(LIBDIR)
else
	LIBS += -lGL
	LDFLAGS += -Wl,-rpath,'$$ORIGIN/../../$(LIBDIR)'
endif

# LuaRocks installation directories (can be overridden)
INST_PREFIX ?= /usr/local
INST_BINDIR ?= $(INST_PREFIX)/bin
INST_LIBDIR ?= $(INST_PREFIX)/lib/lua/5.4
INST_LUADIR ?= $(INST_PREFIX)/share/lua/5.4

.PHONY: default all CLEAN directories unite buildlib app platform example install

default: directories modules unite
all: default
build: default

/%:
	find ${SOURCEDIR}$@ -name "*.c" | sed 's/.*/#include "&"/' | $(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)$@.o -

platform:
	$(MAKE) -C $(PLATFORM_LIBDIR) OUTDIR=../../$(LIBDIR)

buildunite: clean $(SOURCEMODULES2)

buildlib: platform
ifeq ($(shell uname -s),Darwin)
	$(CC) $(addprefix ${OBJECTDIR}/,$(UNITEOBJECTS)) -shared -Wall $(LIBS) -o $(TARGETLIB) $(LDFLAGS) -Wl,-rpath,@loader_path
else
	$(CC) $(addprefix ${OBJECTDIR}/,$(UNITEOBJECTS)) -shared -Wall $(LIBS) -o $(TARGETLIB) $(LDFLAGS) -Wl,-rpath,'$$ORIGIN'
endif

app: platform
	$(CC) $(CFLAGS) $(SOURCEDIR)/orca.c -Wall $(LIBS) -o $(TARGET) $(LDFLAGS)

unite: directories buildunite buildlib app

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $(addprefix $(OBJECTDIR)/,$(notdir $@))

directories:
	mkdir -p ${OBJECTDIR}
	mkdir -p ${BINDIR}
	mkdir -p ${LIBDIR}
	mkdir -p ${SHAREDIR}

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(addprefix ${OBJECTDIR}/,$(notdir $(OBJECTS))) -Wall $(LIBS) -o $@ $(LDFLAGS)

# paktool:
# 	gcc -o ${BINDIR}/paktool -I. ${SOURCEDIR}/targets/paktool.c

# writepaklist:
# 	echo

# buildpaks: paktool
# 	${BINDIR}/paktool $(SHAREDIR)/pak0.pak $(PAKS)

clean:
	-rm -f $(OBJECTDIR)/*.o
	-rm -f $(SHAREDIR)/*.pak
	-rm -f $(BINDIR)/paktool
	-rm -f $(BINDIR)/$(APPNAME)
	-rm -f $(TARGET)
	$(MAKE) -C $(PLATFORM_LIBDIR) clean

andrun: unite
	$(TARGET) $(DATADIR)

run:
	$(TARGET) $(DATADIR)

example:
	$(TARGET) samples/Example

debug:
	gdb -ex "run $(DATADIR)" -ex "bt" -ex "quit" --args $(TARGET) $(DATADIR)

DATA_DIR = ../icui
PACK_DIR = ../icui_pz2
ICUI_DATA = find $(DATA_DIR) -type f -name "*.xdesc" | while read file; do dirname "$$(dirname "$$file")"; done | sort -u 
# XDESC_DIRS := $(shell find $(DATA_DIR) -type f -name "*.xdesc" | while read file; do dirname $$(dirname $$file); done | sort -u)
XDESC_DIRS := $(shell find $(DATA_DIR) -type f -name "*.xdesc" | while read file; do dirname $$(dirname $$file); done | sort -u | grep -vE "^\\.$$")

font:
	xxd -i "images/vga8x12_extra_chars.png" > "source/renderer/builtins/r_builtin_charset2.c"

pak:
	Build/Debug/pz2 -gz -luac $(XDESC_DIRS)

DATA_DIR = ../icui
PACK_DIR = ../icui_pz2

move_pz2:
	find $(DATA_DIR) -type f -name "*.pz2" | while read file; do \
	    rel_path="$${file#$(DATA_DIR)/}"; \
	    mkdir -p "$(PACK_DIR)/$$(dirname "$$rel_path")"; \
	    mv "$$file" "$(PACK_DIR)/$$(dirname "$$rel_path")/"; \
	done

modules:
#	python3 tools/conv-lua.py presentation
	cd tools && \
	python3 conv-module.py \
	../source/geometry/geom.xml \
	../source/core/core.xml \
	../source/renderer/api/renderer.xml \
	../source/UIKit/UIKit.xml \
	../source/SceneKit/SceneKit.xml \
	../source/filesystem/filesystem.xml && cd ..

fonts:
	python3 cd tools && \
	../images/vga8x12_extra_chars.png \
	../source/renderer/builtin/r_builtin_charset2.c \
	images_vga8x12_extra_chars_png && cd ..

# Install target for LuaRocks
install: all
	mkdir -p $(INST_BINDIR)
	mkdir -p $(INST_LIBDIR)
	mkdir -p $(INST_LUADIR)/orca
	# Install the binary
	install -m 0755 $(TARGET) $(INST_BINDIR)/
	# Install the shared library (rename for Lua's require system)
	install -m 0755 $(TARGETLIB) $(INST_LIBDIR)/orca.so
	# Install the platform library
	install -m 0755 $(LIBDIR)/libplatform.so $(INST_LIBDIR)/
	# Install Lua modules
	install -m 0644 source/core/behaviour.lua $(INST_LUADIR)/orca/behaviour.lua
	install -m 0644 main.lua $(INST_LUADIR)/orca/main.lua

test:
	$(TARGET) tests/test1.lua