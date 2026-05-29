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
MODULES = geometry orca platform sysutil console parsers debug network renderer filesystem core
PLUGINS = $(notdir $(wildcard $(PLUGINDIR)/*))
PLUGINLIBS = $(addprefix $(PLUGINLIBDIR)/,$(addsuffix .so,$(PLUGINS)))
SOURCEMODULES = $(addprefix ${SOURCEDIR}/, $(MODULES))
OBJECTS = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(shell find $(dir) -name "*.c" 2>/dev/null)))
HEADERS = $(wildcard *.h)
SOURCEMODULES2 = $(addprefix /, $(MODULES))
UNITEOBJECTS = $(addsuffix .o, $(MODULES))
UNITE = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(shell find $(dir) -name "*.c" 2>/dev/null)))
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

ifeq ($(V),1)
Q =
else
Q = @
endif

.PHONY: default all CLEAN directories unite buildlib buildplugins app platform example weather install codegen-host codegen-clean codegen-sample modules modules-c-preview modules-c-diff test test-headless test-properties test-styles test-filesystem test-message-registry test-trigger-actions test-editor test-text-layout test-stack-layout test-grid-layout test-interaction test-node test-state-manager test-animations test-timers test-styles-lua test-body test-console-view test-widget test-router test-application test-geometry test-parsers test-object-hierarchy test-object-retention test-async test-tabbar test-tab-interaction test-layout

default: directories modules unite
all: default
build: default

/%: | modules directories
	$(Q)echo "Compiling $(notdir $@)"
	$(Q)find ${SOURCEDIR}$@ -name "*.c" 2>/dev/null | sed 's/.*/#include "&"/' | $(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)$@.o -

platform: | directories
	$(Q)$(MAKE) -C $(PLATFORM_LIBDIR) OUTDIR=../../$(LIBDIR) ARCH_FLAGS="$(ARCH_FLAGS)"

$(SOURCEMODULES2): | directories

buildunite: modules $(SOURCEMODULES2)

buildlib: platform buildunite
ifeq ($(shell uname -s),Darwin)
	$(Q)echo "Linking $(notdir $(TARGETLIB))"
	$(Q)$(CC) $(addprefix ${OBJECTDIR}/,$(UNITEOBJECTS)) -shared -Wall $(LIBS) -o $(TARGETLIB) $(LDFLAGS) -Wl,-rpath,@loader_path
else
	$(Q)echo "Linking $(notdir $(TARGETLIB))"
	$(Q)$(CC) $(addprefix ${OBJECTDIR}/,$(UNITEOBJECTS)) -shared -Wall $(LIBS) -o $(TARGETLIB) $(LDFLAGS) -Wl,-rpath,'$$ORIGIN'
endif

buildplugins: buildlib $(PLUGINLIBS)

$(PLUGINLIBDIR)/%.so: buildlib | directories
	$(Q)mkdir -p $(PLUGINLIBDIR)
	$(Q)echo "Building plugin $*"
ifeq ($(shell uname -s),Darwin)
	$(Q)find $(PLUGINDIR)/$* -name "*.c" | sed 's/.*/#include "&"/' | \
		$(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)/plugin_$*.o - && \
		$(CC) $(OBJECTDIR)/plugin_$*.o -shared -Wall $(LIBS) -lorca -o $@ $(LDFLAGS) -Wl,-rpath,@loader_path
else
	$(Q)find $(PLUGINDIR)/$* -name "*.c" | sed 's/.*/#include "&"/' | \
		$(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)/plugin_$*.o - && \
		$(CC) $(OBJECTDIR)/plugin_$*.o -shared -Wall $(LIBS) -lorca -o $@ $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/..'
endif

app: modules platform
	$(Q)echo "Linking $(APPNAME)"
	$(Q)$(CC) $(CFLAGS) $(SOURCEDIR)/orca.c -Wall $(LIBS) -o $(TARGET) $(LDFLAGS)

unite: directories buildunite buildlib buildplugins app copyshare

%.o: %.c $(HEADERS) | modules
	$(Q)$(CC) $(CFLAGS) -c $< -o $(addprefix $(OBJECTDIR)/,$(notdir $@))

directories:
	$(Q)mkdir -p ${OBJECTDIR}
	$(Q)mkdir -p ${BINDIR}
	$(Q)mkdir -p ${LIBDIR}
	$(Q)mkdir -p ${PLUGINLIBDIR}
	$(Q)mkdir -p ${SHAREDIR}

codegen-host:
	$(Q)$(MAKE) -f tools/Makefile MODULE_ROOT= codegen-host

codegen-sample:
	$(Q)$(MAKE) -f tools/Makefile MODULE_ROOT= codegen-sample

codegen-clean:
	$(Q)$(MAKE) -f tools/Makefile MODULE_ROOT= codegen-clean

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
	$(MAKE) codegen-clean
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
	$(Q)$(MAKE) -f tools/Makefile MODULE_ROOT=

modules-c-preview:
	$(Q)$(MAKE) -f tools/Makefile MODULE_ROOT= modules-c-preview

modules-c-diff:
	$(Q)$(MAKE) -f tools/Makefile MODULE_ROOT= modules-c-diff

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
TEST_FILESYSTEM_BIN = $(BINDIR)/test_filesystem
TEST_MESSAGE_REGISTRY_BIN = $(BINDIR)/test_message_registry
TEST_TRIGGER_ACTIONS_BIN = $(BINDIR)/test_trigger_actions
TEST_EDITOR_BIN = $(BINDIR)/test_editor
TEST_LDFLAGS = $(subst $$ORIGIN,$$$$ORIGIN,$(LDFLAGS)) -lorca -ldl -lpthread
EDITOR_PLUGIN_OBJECT = $(OBJECTDIR)/plugin_EditorKit.o
EDITOR_PLUGIN_SOURCES = $(shell find $(PLUGINDIR)/EditorKit -name "*.c" 2>/dev/null)

$(EDITOR_PLUGIN_OBJECT): $(EDITOR_PLUGIN_SOURCES) buildlib | directories
	$(Q)echo "Compiling plugin EditorKit"
	$(Q)find $(PLUGINDIR)/EditorKit -name "*.c" | sed 's/.*/#include "&"/' | $(CC) $(CFLAGS) -x c -c -o $@ -

define C_TEST_RULE
$(1): $(4)
	$(Q)echo "Building $(1)"
	$(Q)$(CC) $(CFLAGS) $(5) -Wall $(2) -o $(3) $(6)
	$(Q)$(3)
endef

define LUA_TEST_RULE
$(1): $(3)
	$(Q)echo "Running $(1)"
	$(Q)$(TARGET) -test=$(2)
endef

$(eval $(call C_TEST_RULE,test-properties,tests/test_properties.c,$(TEST_PROPERTIES_BIN),platform $(SOURCEMODULES2) buildlib,-DTEST_MEMORY,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-styles,tests/test_styles.c,$(TEST_STYLES_BIN),platform $(SOURCEMODULES2) buildlib,-DTEST_MEMORY,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-filesystem,tests/test_filesystem.c,$(TEST_FILESYSTEM_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-message-registry,tests/test_message_registry.c,$(TEST_MESSAGE_REGISTRY_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-trigger-actions,tests/test_trigger_actions.c,$(TEST_TRIGGER_ACTIONS_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-editor,tests/test_editor.c $(EDITOR_PLUGIN_OBJECT),$(TEST_EDITOR_BIN),$(EDITOR_PLUGIN_OBJECT),,$(TEST_LDFLAGS) -lplatform -lm))

HEADLESS_LUA_TESTS = test-layout test-state-manager test-animations test-timers test-styles-lua test-body test-console-view test-object-retention test-async test-widget test-router test-application test-url-for test-geometry test-parsers test-object-hierarchy test-tabbar test-tab-interaction test-text-layout test-stack-layout test-grid-layout test-interaction test-node

$(eval $(call LUA_TEST_RULE,test-layout,tests/test_layout.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-state-manager,tests/test_state_manager.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-animations,tests/test_animations.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-timers,tests/test_timers.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-styles-lua,tests/test_styles_lua.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-body,tests/test_body.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-console-view,tests/test_console_view.lua,unite))
$(eval $(call LUA_TEST_RULE,test-object-retention,tests/test_object_retention.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-async,tests/test_async.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-widget,tests/widget_spec.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-router,tests/router_spec.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-application,tests/application_spec.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-url-for,tests/url_for_spec.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-geometry,tests/test_geometry.lua,unite))
$(eval $(call LUA_TEST_RULE,test-parsers,tests/test_parsers.lua,unite))
$(eval $(call LUA_TEST_RULE,test-object-hierarchy,tests/test_object_hierarchy.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-tabbar,tests/test_tabbar.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-tab-interaction,tests/test_tab_interaction.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-text-layout,tests/test_text_layout.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-stack-layout,tests/test_stack_layout.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-grid-layout,tests/test_grid_layout.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-interaction,tests/test_interaction.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-node,tests/test_node.lua,app copyshare))

test-headless: unite test-properties test-styles test-filesystem test-message-registry test-trigger-actions test-editor $(HEADLESS_LUA_TESTS)

test: test-headless test-text-layout test-stack-layout test-grid-layout test-interaction test-tab-interaction test-node
	$(Q)echo "Running test"
	$(Q)$(TARGET) -test=tests/test.xml

include Makefile.webgl
