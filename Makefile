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
UNAME_S := $(shell uname -s)
HOST_ARCH := $(shell uname -m)
ifeq ($(UNAME_S),Darwin)
DARWIN_ARM_CAPABLE := $(shell sysctl -in hw.optional.arm64 2>/dev/null)
ifeq ($(DARWIN_ARM_CAPABLE),1)
ARCH ?= arm64
else
ARCH ?= $(HOST_ARCH)
endif
else
ARCH ?= $(HOST_ARCH)
endif
ARCH_FLAGS :=

ifeq ($(UNAME_S),Darwin)
ifeq ($(ARCH),arm64)
ARCH_FLAGS += -arch arm64
else ifeq ($(ARCH),x86_64)
ARCH_FLAGS += -arch x86_64
endif
endif

# Allow CFLAGS to be passed from luarocks, but ensure we have base flags
CFLAGS ?= -O2 -g
# Always add these flags, even if CFLAGS is passed from outside
CFLAGS += -fpic -I. -I$(CURDIR)
LDFLAGS = -L$(LIBDIR)
CFLAGS += $(ARCH_FLAGS)
LDFLAGS += $(ARCH_FLAGS)
MODULES = geometry orca platform sysutil console parsers debug network renderer filesystem core vsomeip
PLUGINS = $(notdir $(wildcard $(PLUGINDIR)/*))
SOURCEMODULES = $(addprefix ${SOURCEDIR}/, $(MODULES))
OBJECTS = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(wildcard $(dir)/*.c)))
HEADERS = $(wildcard *.h)
SOURCEMODULES2 = $(addprefix /, $(MODULES))
UNITEOBJECTS = $(addsuffix .o, $(MODULES))
UNITE = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(wildcard $(dir)/*.c)))
CFLAGS += $(shell pkg-config --cflags zlib liblz4 lua5.4 libjpeg freetype2 libxml-2.0 2>/dev/null || pkg-config --cflags zlib liblz4 lua libjpeg freetype2 libxml-2.0 2>/dev/null)
LDFLAGS += $(shell pkg-config --libs zlib liblz4 lua5.4 freetype2 libjpeg libpng libxml-2.0 2>/dev/null || pkg-config --libs zlib liblz4 lua freetype2 libjpeg libpng libxml-2.0 2>/dev/null)

ifeq ($(UNAME_S),Darwin)
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

.PHONY: default all CLEAN directories unite buildlib buildplugins app platform example weather install test test-headless test-properties test-styles test-state-manager test-animations test-timers test-styles-lua test-body test-console-view test-widget test-router test-application

default: directories unite
all: default
build: default

/%:
	find ${SOURCEDIR}$@ -name "*.c" | sed 's/.*/#include "&"/' | $(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)$@.o -

platform:
	$(MAKE) -C $(PLATFORM_LIBDIR) OUTDIR=../../$(LIBDIR) ARCH_FLAGS="$(ARCH_FLAGS)"

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
	$(MAKE) -C $(PLATFORM_LIBDIR) OUTDIR=../../$(LIBDIR) clean

andrun: unite
	$(TARGET) $(DATADIR)

run:
	$(TARGET) $(DATADIR)

example:
	$(TARGET) samples/Example

weather: copyshare
	@if [ ! -x $(TARGET) ]; then \
		echo "Missing $(TARGET). Build the app before running 'make weather'."; \
		exit 1; \
	fi
	$(TARGET) samples/Weather

debug:
	gdb -ex "run $(DATADIR)" -ex "bt" -ex "quit" --args $(TARGET) $(DATADIR)

DATA_DIR = ../icui
PACK_DIR = ../icui_pz2
ICUI_DATA = find $(DATA_DIR) -type f -name "*.xdesc" | while read file; do dirname "$$(dirname "$$file")"; done | sort -u 
# XDESC_DIRS := $(shell find $(DATA_DIR) -type f -name "*.xdesc" | while read file; do dirname $$(dirname $$file); done | sort -u)
XDESC_DIRS = $(shell find $(DATA_DIR) -type f -name "*.xdesc" | while read file; do dirname $$(dirname $$file); done | sort -u | grep -vE "^\\.$$")

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

TEST_PROPERTIES_BIN = $(BINDIR)/test_properties
TEST_STYLES_BIN = $(BINDIR)/test_styles
TEST_LDFLAGS = $(LDFLAGS) -lorca -ldl -lpthread

test-properties: platform $(SOURCEMODULES2) buildlib
	$(CC) $(CFLAGS) -DTEST_MEMORY -Wall tests/test_properties.c -o $(TEST_PROPERTIES_BIN) $(TEST_LDFLAGS)
	$(TEST_PROPERTIES_BIN)

test-styles: platform $(SOURCEMODULES2) buildlib
	$(CC) $(CFLAGS) -DTEST_MEMORY -Wall tests/test_styles.c -o $(TEST_STYLES_BIN) $(TEST_LDFLAGS)
	$(TEST_STYLES_BIN)

test-state-manager: app copyshare
	$(TARGET) -test=tests/test_state_manager.lua

test-animations: app copyshare
	$(TARGET) -test=tests/test_animations.lua

test-timers: app copyshare
	$(TARGET) -test=tests/test_timers.lua

test-styles-lua: app copyshare
	$(TARGET) -test=tests/test_styles_lua.lua

test-body: app copyshare
	$(TARGET) -test=tests/test_body.lua

test-console-view: unite
	$(TARGET) -test=tests/test_console_view.lua

test-widget: app copyshare
	$(TARGET) -test=tests/widget_spec.moon

test-router: app copyshare
	$(TARGET) -test=tests/router_spec.moon

test-application: app copyshare
	$(TARGET) -test=tests/application_spec.moon

test-loadview: app copyshare
	$(TARGET) -test=tests/loadview_spec.moon

test: test-headless test-properties test-styles
	$(TARGET) -test=tests/test1.lua
	$(TARGET) -test=tests/test.xml

test-headless: unite test-properties test-styles
	$(TARGET) -test=tests/test_layout.lua
	$(TARGET) -test=tests/test_state_manager.lua
	$(TARGET) -test=tests/test_animations.lua
	$(TARGET) -test=tests/test_timers.lua
	$(TARGET) -test=tests/test_styles_lua.lua
	$(TARGET) -test=tests/test_body.lua
	$(TARGET) -test=tests/test_console_view.lua
	$(TARGET) -test=tests/widget_spec.moon
	$(TARGET) -test=tests/router_spec.moon
	$(TARGET) -test=tests/application_spec.moon
	$(TARGET) -test=tests/loadview_spec.moon

include Makefile.webgl