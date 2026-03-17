APPNAME = orca
LIBNAME = orca
BUILDDIR = build
SOURCEDIR = source
PLUGINDIR = plugins
DATADIR = data
RESOURCEDIR = share
OBJECTDIR = $(BUILDDIR)/obj
SHAREDIR = $(BUILDDIR)/share
BINDIR = $(BUILDDIR)/bin
LIBDIR = $(BUILDDIR)/lib
PLUGINLIBDIR = $(LIBDIR)/liborca
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
MODULES = geometry orca platform sysutil console localization parsers debug network renderer filesystem core vsomeip server editor backend
PLUGINS = $(notdir $(wildcard $(PLUGINDIR)/*))
SOURCEMODULES = $(addprefix ${SOURCEDIR}/, $(MODULES))
OBJECTS = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(wildcard $(dir)/*.c)))
HEADERS = $(wildcard *.h)
SOURCEMODULES2 = $(addprefix /, $(MODULES))
UNITEOBJECTS = $(addsuffix .o, $(MODULES))
UNITE = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(wildcard $(dir)/*.c)))
CFLAGS += $(shell pkg-config --cflags zlib liblz4 lua5.4 libjpeg freetype2 libxml-2.0 2>/dev/null)
LDFLAGS += $(shell pkg-config --libs zlib liblz4 lua5.4 freetype2 libjpeg libpng libxml-2.0 2>/dev/null)

ifeq ($(shell uname -s),Darwin)
	CFLAGS += -DGL_SILENCE_DEPRECATION
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
INST_SHAREDIR ?= $(INST_PREFIX)/share/orca

# ─── WebGL / Emscripten build ─────────────────────────────────────────────
# Build all C modules into a single .html/.js/.wasm output using emcc.
#
# Prerequisites (install once via emsdk):
#   git clone https://github.com/emscripten-core/emsdk.git
#   cd emsdk && ./emsdk install latest && ./emsdk activate latest
#   source ./emsdk_env.sh
#
# Additional libraries that must be compiled for WASM with emcc first:
#   - lua5.4  : build from https://www.lua.org/download.html with emcc
#   - libxml2 : build from https://gitlab.gnome.org/GNOME/libxml2 with emcc
#   - liblz4  : build from https://github.com/lz4/lz4 with emcc
#
# Libraries available as Emscripten ports (no manual build needed):
#   zlib, libpng, libjpeg, freetype  -- provided via -sUSE_* flags below.
#
# Usage:
#   make webgl                          # build without bundled data
#   make webgl WEBGL_DATA=samples/Example  # bundle a project into the build
# ──────────────────────────────────────────────────────────────────────────

WEBGL_DIR      = build/webgl
WEBGL_DATA    ?=
WEBGL_EMCC     = emcc

# Modules included in the WebGL build.  Excluded: network (curl/sockets),
# vsomeip (C++ service discovery), server (host-only), editor (desktop UI).
WEBGL_MODULES  = geometry orca sysutil console localization parsers debug \
                 renderer filesystem core backend
WEBGL_SRCMODS  = $(addprefix $(SOURCEDIR)/, $(WEBGL_MODULES))
WEBGL_SOURCES  = $(foreach d, $(WEBGL_SRCMODS), $(wildcard $(d)/*.c))

# Platform sources compiled directly into the binary (no SIDE_MODULE needed).
WEBGL_PLAT_SRC = $(wildcard $(PLATFORM_LIBDIR)/webgl/*.c)

WEBGL_CFLAGS  = -O2 -g -I. -I$(CURDIR) -I$(PLATFORM_LIBDIR) \
                -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_FREETYPE=1 -sUSE_LIBJPEG=1 \
                -sUSE_WEBGL2=1 -sMIN_WEBGL_VERSION=1 -sMAX_WEBGL_VERSION=2 \
                -sASYNCIFY=1 -sASYNCIFY_IMPORTS='["emscripten_sleep"]' \
                -sALLOW_MEMORY_GROWTH=1

WEBGL_LDFLAGS = -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_FREETYPE=1 -sUSE_LIBJPEG=1 \
                -sUSE_WEBGL2=1 -sMIN_WEBGL_VERSION=1 -sMAX_WEBGL_VERSION=2 \
                -sASYNCIFY=1 -sASYNCIFY_IMPORTS='["emscripten_sleep"]' \
                -sALLOW_MEMORY_GROWTH=1 \
                -sEXPORTED_RUNTIME_METHODS='["ccall","cwrap","FS"]' \
                -sEXIT_RUNTIME=0

# ─── WASM dependency builds ───────────────────────────────────────────────
# Builds lua5.4, libxml2, and liblz4 as WASM static libraries.
# Run once before `make webgl` (with emcc in PATH):
#   emmake make wasm-deps
# Override WASM_DEPS_DIR to install into a custom prefix.
# ──────────────────────────────────────────────────────────────────────────
WASM_DEPS_DIR  ?= build/wasm-deps
LUA_VERSION     = 5.4.7
LZ4_VERSION     = 1.10.0
XML2_VERSION    = 2.9.14

# Add WASM dep headers / libraries to the WebGL compiler and linker flags.
WEBGL_CFLAGS  += -I$(WASM_DEPS_DIR)/include
WEBGL_LDFLAGS += -L$(WASM_DEPS_DIR)/lib -llua5.4 -lxml2 -llz4

# Bundle data directory into the VFS when WEBGL_DATA is set.
ifneq ($(WEBGL_DATA),)
WEBGL_LDFLAGS += --preload-file $(WEBGL_DATA)@/$(WEBGL_DATA)
endif

.PHONY: default all CLEAN directories unite buildlib buildplugins app platform example install webgl \
        wasm-deps wasm-lua wasm-lz4 wasm-xml2

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

buildplugins: buildlib
	mkdir -p $(PLUGINLIBDIR)
ifeq ($(shell uname -s),Darwin)
	$(foreach p,$(PLUGINS), \
		find $(PLUGINDIR)/$(p) -name "*.c" | sed 's/.*/#include "&"/' | \
		$(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)/plugin_$(p).o - && \
		$(CC) $(OBJECTDIR)/plugin_$(p).o -shared -Wall $(LIBS) -lorca -o $(PLUGINLIBDIR)/$(p).so $(LDFLAGS) -Wl,-rpath,@loader_path || exit 1;)
else
	$(foreach p,$(PLUGINS), \
		find $(PLUGINDIR)/$(p) -name "*.c" | sed 's/.*/#include "&"/' | \
		$(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)/plugin_$(p).o - && \
		$(CC) $(OBJECTDIR)/plugin_$(p).o -shared -Wall $(LIBS) -lorca -o $(PLUGINLIBDIR)/$(p).so $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/..' || exit 1;)
endif

app: platform
	$(CC) $(CFLAGS) $(SOURCEDIR)/orca.c -Wall $(LIBS) -o $(TARGET) $(LDFLAGS)

unite: directories buildunite buildlib buildplugins app copyshare

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $(addprefix $(OBJECTDIR)/,$(notdir $@))

directories:
	mkdir -p ${OBJECTDIR}
	mkdir -p ${BINDIR}
	mkdir -p ${LIBDIR}
	mkdir -p ${PLUGINLIBDIR}
	mkdir -p ${SHAREDIR}

copyshare:
	mkdir -p ${SHAREDIR}
	cp -r ${RESOURCEDIR}/* ${SHAREDIR}/

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
	-rm -rf $(SHAREDIR)/*
	-rm -rf $(PLUGINLIBDIR)
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
	$(MAKE) -j8 -C tools

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
	mkdir -p $(INST_SHAREDIR)
	# Install the binary
	install -m 0755 $(TARGET) $(INST_BINDIR)/
	# Install the shared library (rename for Lua's require system)
	install -m 0755 $(TARGETLIB) $(INST_LIBDIR)/orca.so
	# Install the platform library
	install -m 0755 $(LIBDIR)/libplatform.so $(INST_LIBDIR)/
	# Install Lua modules
	install -m 0644 source/core/behaviour.lua $(INST_LUADIR)/orca/behaviour.lua
	install -m 0644 main.lua $(INST_LUADIR)/orca/main.lua
	# Install shared data files (fonts, icons, Lua plugins)
	cp -r $(RESOURCEDIR)/* $(INST_SHAREDIR)/

test:
	$(TARGET) -test=tests/test1.lua
	$(TARGET) -test=tests/test.xml

# ─── WebGL build target ────────────────────────────────────────────────────
# Compiles all engine modules into a self-contained orca.html + orca.js +
# orca.wasm triple using Emscripten.  See the WEBGL section at the top of
# this Makefile for a full list of prerequisites and usage notes.
webgl: $(WEBGL_DIR) $(LIBDIR)
	$(WEBGL_EMCC) $(WEBGL_CFLAGS) \
		$(WEBGL_PLAT_SRC) \
		$(WEBGL_SOURCES) \
		$(SOURCEDIR)/orca.c \
		$(WEBGL_LDFLAGS) \
		-o $(WEBGL_DIR)/orca.html

$(WEBGL_DIR):
	mkdir -p $(WEBGL_DIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)

# ─── WASM dependency build recipes ────────────────────────────────────────

wasm-deps: wasm-lua wasm-lz4 wasm-xml2

wasm-lua: $(WASM_DEPS_DIR)/lib/liblua5.4.a

$(WASM_DEPS_DIR)/lib/liblua5.4.a:
	mkdir -p $(WASM_DEPS_DIR)/include/lua5.4 $(WASM_DEPS_DIR)/lib
	wget -qO /tmp/lua-$(LUA_VERSION).tar.gz \
	    https://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	rm -rf /tmp/wasm-lua && mkdir -p /tmp/wasm-lua
	tar xzf /tmp/lua-$(LUA_VERSION).tar.gz -C /tmp/wasm-lua --strip-components=1
	cd /tmp/wasm-lua/src && emcc -O2 -DLUA_USE_POSIX -I. -c \
	    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c \
	    llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c \
	    ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c \
	    lcorolib.c ldblib.c liolib.c lmathlib.c loadlib.c loslib.c \
	    lstrlib.c ltablib.c lutf8lib.c linit.c
	cd /tmp/wasm-lua/src && emar rcs liblua5.4.a *.o
	cp /tmp/wasm-lua/src/lua.h /tmp/wasm-lua/src/luaconf.h \
	   /tmp/wasm-lua/src/lauxlib.h /tmp/wasm-lua/src/lualib.h \
	   $(WASM_DEPS_DIR)/include/lua5.4/
	cp /tmp/wasm-lua/src/liblua5.4.a $(WASM_DEPS_DIR)/lib/

wasm-lz4: $(WASM_DEPS_DIR)/lib/liblz4.a

$(WASM_DEPS_DIR)/lib/liblz4.a:
	mkdir -p $(WASM_DEPS_DIR)/include $(WASM_DEPS_DIR)/lib
	wget -qO /tmp/lz4-$(LZ4_VERSION).tar.gz \
	    https://github.com/lz4/lz4/archive/refs/tags/v$(LZ4_VERSION).tar.gz
	rm -rf /tmp/wasm-lz4 && mkdir -p /tmp/wasm-lz4
	tar xzf /tmp/lz4-$(LZ4_VERSION).tar.gz -C /tmp/wasm-lz4 --strip-components=1
	cd /tmp/wasm-lz4/lib && emcc -O2 -c lz4.c lz4hc.c lz4frame.c xxhash.c
	cd /tmp/wasm-lz4/lib && emar rcs liblz4.a lz4.o lz4hc.o lz4frame.o xxhash.o
	cp /tmp/wasm-lz4/lib/lz4.h /tmp/wasm-lz4/lib/lz4hc.h \
	   /tmp/wasm-lz4/lib/lz4frame.h $(WASM_DEPS_DIR)/include/
	cp /tmp/wasm-lz4/lib/liblz4.a $(WASM_DEPS_DIR)/lib/

wasm-xml2: $(WASM_DEPS_DIR)/lib/libxml2.a

$(WASM_DEPS_DIR)/lib/libxml2.a:
	mkdir -p $(WASM_DEPS_DIR) /tmp/wasm-xml2
	wget -qO /tmp/libxml2-$(XML2_VERSION).tar.xz \
	    https://download.gnome.org/sources/libxml2/2.9/libxml2-$(XML2_VERSION).tar.xz
	tar xJf /tmp/libxml2-$(XML2_VERSION).tar.xz -C /tmp/wasm-xml2 --strip-components=1
	cd /tmp/wasm-xml2 && emconfigure ./configure \
	    --prefix=$(abspath $(WASM_DEPS_DIR)) \
	    --disable-shared --enable-static \
	    --without-python --without-http --without-ftp \
	    --without-zlib --without-lzma --without-iconv
	cd /tmp/wasm-xml2 && emmake make install-strip