APPNAME = orca
ifndef SCENE_FORMAT
SCENE_FORMAT := $(if $(findstring -DORCA_USE_LISP,$(CPPFLAGS) $(CFLAGS) $(CC)),lisp,xml)
endif
ifeq ($(SCENE_FORMAT),lisp)
SCENE_FORMAT_FLAGS = -DORCA_USE_LISP
SCENE_FORMAT_EXT = .lisp
else ifeq ($(SCENE_FORMAT),xml)
ifneq ($(findstring -DORCA_USE_LISP,$(CPPFLAGS) $(CFLAGS) $(CC)),)
$(error SCENE_FORMAT=xml conflicts with -DORCA_USE_LISP)
endif
SCENE_FORMAT_FLAGS =
SCENE_FORMAT_EXT = .xml
else
$(error SCENE_FORMAT must be lisp or xml)
endif
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
ifneq ($(filter no-lz4,$(MAKECMDGOALS)),)
CFLAGS += -DORCA_NO_LZ4
endif
# Always add these flags, even if CFLAGS is passed from outside
CFLAGS += -fpic -I. -I$(CURDIR) -Igenerated
LDFLAGS = -L$(LIBDIR)
CFLAGS += $(SANITIZER_FLAGS)
LDFLAGS += $(SANITIZER_FLAGS)
CFLAGS += $(ARCH_FLAGS)
LDFLAGS += $(ARCH_FLAGS)
MODULES = geometry orca platform sysutil console parsers debug network renderer filesystem core data media credentials notifications
PLUGINS = $(notdir $(wildcard $(PLUGINDIR)/*))
PLUGINLIBS = $(addprefix $(PLUGINLIBDIR)/,$(addsuffix .so,$(PLUGINS)))
PLUGIN_DEPS_SceneKit = $(PLUGINLIBDIR)/UIKit.so
SOURCEMODULES = $(addprefix ${SOURCEDIR}/, $(MODULES))
OBJECTS = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(shell find $(dir) -name "*.c" 2>/dev/null)))
HEADERS = $(wildcard *.h)
SOURCEMODULES2 = $(addprefix /, $(MODULES))
UNITEOBJECTS = $(addsuffix .o, $(MODULES))
UNITE = $(patsubst %.c, %.o, $(foreach dir,$(SOURCEMODULES),$(shell find $(dir) -name "*.c" 2>/dev/null)))
LUA_PKG := $(shell pkg-config --exists lua5.4 && echo lua5.4 || echo lua)
PKG_DEPS := zlib $(LUA_PKG) libxml-2.0
ifeq ($(filter -DORCA_NO_LZ4,$(CFLAGS)),)
PKG_DEPS += liblz4
endif
# Keep available flags when another dependency is missing, and show its diagnostic.
PKG_CFLAGS := $(foreach dep,$(PKG_DEPS),$(shell pkg-config --cflags $(dep)))
PKG_LIBS := $(foreach dep,$(PKG_DEPS),$(shell pkg-config --libs $(dep)))
CFLAGS += $(PKG_CFLAGS)
LDFLAGS += $(PKG_LIBS)

ifeq ($(UNAME_S),Darwin)
	CFLAGS += -DGL_SILENCE_DEPRECATION
	LIBS += -framework OpenGL -framework IOSurface
	LIBS += -framework ImageIO -framework CoreGraphics -framework Foundation -framework Security
	UNITEOBJECTS += media_apple.o
	UNITEOBJECTS += notifications_apple.o
	LIBS += -framework AppKit -framework UserNotifications
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

.PHONY: default all CLEAN directories unite orca-fbx buildlib buildplugins app platform example weather install codegen-host codegen-clean codegen-sample modules modules-c-preview modules-c-diff test test-headless test-properties test-styles test-filesystem test-message-registry test-trigger-actions test-editor test-texture-downscale test-xml-serialization test-text-layout test-stack-layout test-grid-layout test-grid-tracks test-interaction test-adventure-header-hit test-node test-state-manager test-animations test-timers test-styles-lua test-body test-console-view test-widget test-router test-application test-geometry test-parsers test-object-hierarchy test-object-retention test-async test-tabbar test-tab-interaction test-layout test-listbox

default: directories modules unite
all: default
build: default

.PHONY: no-lz4
no-lz4: default

# Phony: a module target such as /media must not match an existing root directory.
.PHONY: $(SOURCEMODULES2)
$(SOURCEMODULES2): /%: | modules directories
	$(Q)echo "Compiling $(notdir $@)"
	$(Q)find ${SOURCEDIR}$@ -name "*.c" 2>/dev/null | sed 's/.*/#include "&"/' | $(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)$@.o -

platform: | directories
	$(Q)$(MAKE) -C $(PLATFORM_LIBDIR) OUTDIR=../../$(LIBDIR) ARCH_FLAGS="$(ARCH_FLAGS)"

$(SOURCEMODULES2): | directories

buildunite: modules $(SOURCEMODULES2)

ifeq ($(UNAME_S),Darwin)
buildunite: $(OBJECTDIR)/media_apple.o
$(OBJECTDIR)/media_apple.o: source/media/media_apple.m source/media/media_native.h | directories
	$(Q)$(CC) $(CFLAGS) -x objective-c -c $< -o $@
buildunite: $(OBJECTDIR)/notifications_apple.o
$(OBJECTDIR)/notifications_apple.o: source/notifications/notifications_apple.m source/notifications/notifications_native.h | directories
	$(Q)$(CC) $(CFLAGS) -x objective-c -c $< -o $@
endif

buildlib: platform buildunite
ifeq ($(shell uname -s),Darwin)
	$(Q)echo "Linking $(notdir $(TARGETLIB))"
	$(Q)$(CC) $(addprefix ${OBJECTDIR}/,$(UNITEOBJECTS)) -shared -Wall $(LIBS) -o $(TARGETLIB) $(LDFLAGS) -Wl,-rpath,@loader_path
else
	$(Q)echo "Linking $(notdir $(TARGETLIB))"
	$(Q)$(CC) $(addprefix ${OBJECTDIR}/,$(UNITEOBJECTS)) -shared -Wall $(LIBS) -o $(TARGETLIB) $(LDFLAGS) -Wl,-rpath,'$$ORIGIN'
endif

.SECONDEXPANSION:
buildplugins: buildlib $(PLUGINLIBS)

$(PLUGINLIBDIR)/%.so: buildlib $$(PLUGIN_DEPS_$$*) | directories
	$(Q)mkdir -p $(PLUGINLIBDIR)
	$(Q)echo "Building plugin $*"
ifeq ($(shell uname -s),Darwin)
	$(Q)find $(PLUGINDIR)/$* -name "*.c" | sed 's/.*/#include "&"/' | \
		$(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)/plugin_$*.o - && \
		$(CC) $(OBJECTDIR)/plugin_$*.o -shared -Wall $(LIBS) -lorca $(PLUGIN_EXTRA_LIBS_$*) -o $@ $(LDFLAGS) -Wl,-rpath,@loader_path
else
	$(Q)find $(PLUGINDIR)/$* -name "*.c" | sed 's/.*/#include "&"/' | \
		$(CC) $(CFLAGS) -x c -c -o $(OBJECTDIR)/plugin_$*.o - && \
		$(CC) $(OBJECTDIR)/plugin_$*.o -shared -Wall $(LIBS) -lorca $(PLUGIN_EXTRA_LIBS_$*) -o $@ $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/..'
endif

app: modules platform
	$(Q)echo "Linking $(APPNAME)"
	$(Q)$(CC) $(CFLAGS) $(SOURCEDIR)/orca.c -Wall $(LIBS) -o $(TARGET) $(LDFLAGS)

unite: directories buildunite buildlib buildplugins app copyshare

# FBX -> .mesh converter used by tools/blender-export.py. Optional: needs the Autodesk FBX SDK.
FBX_SDK ?= /Applications/Autodesk/FBX SDK/2020.2
orca-fbx: directories
	$(CXX) -std=c++17 -O2 -w -I. -Igenerated -I"$(FBX_SDK)/include" tools/fbx-import.cpp \
		"$(FBX_SDK)/lib/clang/release/libfbxsdk.a" -framework CoreFoundation -liconv -lxml2 -lz \
		-o $(BINDIR)/orca-fbx

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
	$(TARGET) apps/Example

# Instrument cluster demo; Left/Right switch clusters. START=voltage opens on Voltage.
cluster: unite
	CLUSTER_DEMO_START=$(START) $(TARGET) apps/cluster-demo

weather: copyshare
	@if [ ! -x $(TARGET) ]; then \
		echo "Missing $(TARGET). Build the app before running 'make weather'."; \
		exit 1; \
	fi
	$(TARGET) apps/Book

.PHONY: chat test-chat test-chat-ui test-chat-appwrite test-network-headers test-octopus-cache
chat: copyshare
	$(TARGET) apps/chat

test-chat: copyshare
	$(LUA) tests/test_chat_push.lua
	$(TARGET) -test=tests/test_chat_diagnostics.lua
	$(TARGET) -test=tests/test_chat_polling.lua
	$(TARGET) -test=tests/test_chat_sync.lua
	$(TARGET) -test=tests/test_chat_refresh_race.lua
	$(TARGET) -test=tests/test_chat_timeline.lua
	$(TARGET) -test=tests/test_chat_session_store.lua
	$(TARGET) -test=tests/test_chat_api.lua
	$(TARGET) -test=tests/test_chat_appwrite.lua
	$(TARGET) -test=tests/test_chat_mock.lua
	$(TARGET) -test=tests/test_chat_models.lua
	$(TARGET) -test=tests/test_chat_features.lua
	$(TARGET) -test=tests/test_chat_contacts.lua
	$(TARGET) -test=tests/test_chat_app.lua
	$(TARGET) -test=tests/test_chat_loading.lua
	$(TARGET) -test=tests/test_chat_session_ui.lua

# Fast offline presentation checks. Reuse the current binary for app-only edits;
# rebuild separately with `make unite` after native changes or on a fresh checkout.
test-chat-ui: copyshare
	$(LUA) tests/test_chat_photo_policy.lua
	$(TARGET) -test=tests/test_chat_timeline.lua
	$(TARGET) -test=tests/test_chat_photo_loader.lua
	$(TARGET) -test=tests/test_chat_photo_viewer.lua
	$(TARGET) -test=tests/test_footer.lua

# Adapter unit tests need only Lua: no engine startup, resource copy or rebuild.
LUA ?= lua
test-chat-appwrite:
	$(LUA) tests/test_chat_appwrite.lua
	$(LUA) tests/test_chat_push.lua

# Explicitly opted in: real native HTTP and secure session storage against
# Appwrite. Never make this a dependency of test/test-headless/test-chat.
.PHONY: test-chat-live
test-chat-live: copyshare
	$(TARGET) -test=tests/test_chat_live.lua

.PHONY: octopus test-octopus test-octopus-startup-api test-octopus-backend test-octopus-theme
octopus: copyshare
	$(TARGET) apps/octopus

test-octopus: copyshare test-octopus-cache test-octopus-startup-api test-octopus-backend test-octopus-theme
	$(TARGET) -test=tests/test_octopus_api.lua
	$(TARGET) -test=tests/test_octopus_photos.lua
	$(TARGET) -test=tests/test_octopus_templates.lua
	$(TARGET) -test=tests/test_octopus_alignment.lua
	$(TARGET) -test=tests/test_octopus_app.lua

NODE ?= node
test-octopus-theme:
	$(LUA) tests/test_octopus_theme.lua

test-octopus-startup-api:
	$(LUA) tests/test_octopus_startup_api.lua
	$(LUA) tests/test_octopus_pagination.lua

test-octopus-backend:
	$(NODE) --test apps/octopus/backend/test/*.test.js

.PHONY: test-chat-appwrite-backend test-media
test-chat-appwrite-backend:
	$(NODE) --test apps/chat/backend/appwrite/test/*.test.js

# Manual end-to-end checks against the deployed Chat backend; never part of CI or
# `make test`. See apps/chat/backend/appwrite/live/README.md.
CHAT_LIVE_DIR = apps/chat/backend/appwrite/live
.PHONY: test-live test-live-setup test-live-clean test-live-emulated
test-live:
	$(NODE) --test-reporter=spec $(CHAT_LIVE_DIR)/run.js

test-live-setup:
	$(NODE) $(CHAT_LIVE_DIR)/setup.js

test-live-clean:
	$(NODE) $(CHAT_LIVE_DIR)/setup.js --clean

# Offline self-check of the live suite against an in-memory Appwrite emulator.
test-live-emulated:
	$(NODE) --test-reporter=spec $(CHAT_LIVE_DIR)/emulated.js

ifeq ($(UNAME_S),Darwin)
$(BINDIR)/test_media: tests/test_media.m source/media/media.c source/media/media_apple.m source/media/media_native.h modules | directories
	$(CC) $(CFLAGS) tests/test_media.m source/media/media.c source/media/media_apple.m -O2 $(LDFLAGS) -framework Foundation -framework CoreGraphics -framework ImageIO -o $@

test-media: $(BINDIR)/test_media
	$<

test-headless: test-media
endif

# iPhone/iPad .app bundles built directly with the Apple command-line tools.
.PHONY: ios ios-sim ios-run ios-install ios-devices test-ios-tools
ios:
	$(MAKE) -f tools/ios/Makefile SDK=iphoneos app

ios-sim:
	$(MAKE) -f tools/ios/Makefile SDK=iphonesimulator app

ios-run:
	$(MAKE) -f tools/ios/Makefile SDK=iphonesimulator run

ios-install:
	$(MAKE) -f tools/ios/Makefile SDK=iphoneos install

ios-devices:
	$(MAKE) -f tools/ios/Makefile list-devices

test-ios-tools: test-ios-tmpname test-ios-message-frame test-ios-scroll-policy
	PYTHONPYCACHEPREFIX=build/ios/pycache python3 -m unittest discover -s tests -p 'test_ios_tools.py'

test-octopus-cache:
	$(LUA) tests/test_octopus_cache.lua

# Build the engine first with make unite after native changes. The C driver
# receives its loopback fixture URL from Python, so it is not a C_TEST_RULE case.
test-network-headers:
	$(Q)$(CC) $(CFLAGS) -Wall tests/test_network_fetch.c -o $(BINDIR)/test_network_fetch $(TEST_LDFLAGS)
	python3 tests/test_network_headers.py --binary $(TARGET) --native-binary $(BINDIR)/test_network_fetch

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

skills: modules
	$(Q)mkdir -p .agents
	$(Q)cp -r skills/* .agents/

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
TEST_XML_SERIALIZATION_BIN = $(BINDIR)/test_xml_serialization
TEST_SHADER_UNIFORMS_BIN = $(BINDIR)/test_shader_uniforms
.PHONY: test-shader-uniforms
TEST_TEXTURE_DOWNSCALE_BIN = $(BINDIR)/test_texture_downscale
.PHONY: test-texture-downscale
TEST_SCREEN_SPACE_CAPTURE_BIN = $(BINDIR)/test_screen_space_capture
TEST_GL_LIBS := $(if $(filter Darwin,$(UNAME_S)),-framework OpenGL,-lGL)
.PHONY: test-screen-space-capture
TEST_LDFLAGS = $(subst $$ORIGIN,$$$$ORIGIN,$(LDFLAGS)) -lorca -ldl -lpthread
EDITOR_PLUGIN_OBJECT = $(OBJECTDIR)/plugin_EditorKit.o
EDITOR_PLUGIN_SOURCES = $(shell find $(PLUGINDIR)/EditorKit -name "*.c" 2>/dev/null)

$(EDITOR_PLUGIN_OBJECT): $(EDITOR_PLUGIN_SOURCES) buildlib | directories
	$(Q)echo "Compiling plugin EditorKit"
	$(Q)find $(PLUGINDIR)/EditorKit -name "*.c" | sed 's/.*/#include "&"/' | $(CC) $(CFLAGS) -x c -c -o $@ -

# Darwin's two-level namespace bypasses allocator definitions in an executable.
# An injected dylib owns both the interpose tuples and the shared test counter.
ifeq ($(UNAME_S),Darwin)
TEST_MEMORY_DYLIB = $(LIBDIR)/liborca_test_memory.dylib
$(TEST_MEMORY_DYLIB): tests/mem_tracker_darwin.m tests/mem_tracker.h | directories
	$(Q)$(CC) $(CFLAGS) -dynamiclib tests/mem_tracker_darwin.m -framework Foundation -o $@ -Wl,-install_name,$(abspath $(TEST_MEMORY_DYLIB))
endif

define C_TEST_RULE
$(1): $(4) $(if $(filter -DTEST_MEMORY,$(5)),$(TEST_MEMORY_DYLIB))
	$(Q)echo "Building $(1)"
	$(Q)$(CC) $$(CFLAGS) $(5) -Wall $(2) -o $(3) $(6) $(if $(filter -DTEST_MEMORY,$(5)),$(TEST_MEMORY_DYLIB))
	$(Q)$(if $(and $(filter -DTEST_MEMORY,$(5)),$(TEST_MEMORY_DYLIB)),DYLD_INSERT_LIBRARIES="$(abspath $(TEST_MEMORY_DYLIB))") $(3)
endef

define LUA_TEST_RULE
$(1): $(3)
	$(Q)echo "Running $(1)"
	$(Q)$(TARGET) -test=$(2)
endef

.PHONY: test-ios-tmpname
$(eval $(call C_TEST_RULE,test-ios-tmpname,tests/test_ios_tmpname.c,$(BINDIR)/test_ios_tmpname,directories tools/ios/lua_config.h,,))

.PHONY: test-ios-scroll-policy
$(eval $(call C_TEST_RULE,test-ios-scroll-policy,tests/test_ios_scroll_policy.c,$(BINDIR)/test_ios_scroll_policy,directories,,-lm))

ifeq ($(UNAME_S),Darwin)
.PHONY: test-ios-message-frame
$(eval $(call C_TEST_RULE,test-ios-message-frame,tests/test_ios_message_frame.c source/sysutil/queue.c libs/platform/ios/ios_message_queue.m,$(BINDIR)/test_ios_message_frame,unite,-DORCA_IOS,$(TEST_LDFLAGS) -lplatform -framework CoreFoundation))
HEADLESS_IOS_TESTS = test-ios-message-frame
else
.PHONY: test-ios-message-frame
test-ios-message-frame:
	@echo "iOS frame queue integration requires the Apple CoreFoundation framework"
endif

$(eval $(call C_TEST_RULE,test-properties,tests/test_properties.c,$(TEST_PROPERTIES_BIN),platform $(SOURCEMODULES2) buildlib,-DTEST_MEMORY,$(TEST_LDFLAGS)))
STRING_MEMORY_TEST_FLAGS ?= -DTEST_MEMORY
$(eval $(call C_TEST_RULE,test-string-properties,tests/test_string_properties.c,$(BINDIR)/test_string_properties,platform $(SOURCEMODULES2) buildlib,$(STRING_MEMORY_TEST_FLAGS),$(TEST_LDFLAGS)))
.PHONY: test-string-memory test-string-memory-sanitized
test-string-memory: test-string-properties test-string-bindings

test-string-memory-sanitized:
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
		LD_LIBRARY_PATH="$(abspath $(BUILDDIR)/string-sanitized/lib)" $(MAKE) \
		BUILDDIR=$(BUILDDIR)/string-sanitized \
		SANITIZER_FLAGS="-O1 -fsanitize=address,undefined -fno-omit-frame-pointer" \
		STRING_MEMORY_TEST_FLAGS= \
		test-string-memory
$(eval $(call C_TEST_RULE,test-styles,tests/test_styles.c,$(TEST_STYLES_BIN),platform $(SOURCEMODULES2) buildlib,-DTEST_MEMORY,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-filesystem,tests/test_filesystem.c,$(TEST_FILESYSTEM_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-message-registry,tests/test_message_registry.c,$(TEST_MESSAGE_REGISTRY_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-collection-observers,tests/test_collection_observers.c,$(BINDIR)/test_collection_observers,platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-message-payload,tests/test_message_payload.c,$(BINDIR)/test_message_payload,platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS) -lplatform))
$(eval $(call C_TEST_RULE,test-trigger-actions,tests/test_trigger_actions.c,$(TEST_TRIGGER_ACTIONS_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-editor,tests/test_editor.c $(EDITOR_PLUGIN_OBJECT),$(TEST_EDITOR_BIN),$(EDITOR_PLUGIN_OBJECT),,$(TEST_LDFLAGS) -lplatform -lm))
$(eval $(call C_TEST_RULE,test-xml-serialization,tests/test_xml_serialization.c,$(TEST_XML_SERIALIZATION_BIN),platform $(SOURCEMODULES2) buildlib,-DTEST_MEMORY,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-shader-uniforms,tests/test_shader_uniforms.c,$(TEST_SHADER_UNIFORMS_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-texture-downscale,tests/test_texture_downscale.c,$(TEST_TEXTURE_DOWNSCALE_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-screen-space-capture,tests/test_screen_space_capture.c,$(TEST_SCREEN_SPACE_CAPTURE_BIN),platform $(SOURCEMODULES2) buildlib,,$(TEST_LDFLAGS) $(TEST_GL_LIBS)))

# Opt-in desktop test: intentionally excluded from test-headless.
.PHONY: test-macos-retina
ifeq ($(UNAME_S),Darwin)
$(eval $(call C_TEST_RULE,test-macos-retina,tests/test_macos_retina.m,$(BINDIR)/test_macos_retina,unite,-Wno-deprecated-declarations,$(PLUGINLIBDIR)/UIKit.so $(TEST_LDFLAGS) -lplatform -framework AppKit -framework OpenGL))
else
test-macos-retina:
	$(Q)echo "test-macos-retina requires macOS and a desktop display"
endif

$(eval $(call C_TEST_RULE,test-scroll-layout,tests/test_scroll_layout.c,$(BINDIR)/test_scroll_layout,unite,,$(PLUGINLIBDIR)/UIKit.so $(TEST_LDFLAGS) -lplatform -lm))
.PHONY: test-animation-clock
$(eval $(call C_TEST_RULE,test-animation-clock,tests/test_animation_clock.c,$(BINDIR)/test_animation_clock,unite,,$(PLUGINLIBDIR)/UIKit.so $(TEST_LDFLAGS) -lplatform -lm))
.PHONY: test-text-leading
$(eval $(call C_TEST_RULE,test-text-leading,tests/test_text_leading.c,$(BINDIR)/test_text_leading,unite,,$(PLUGINLIBDIR)/UIKit.so $(TEST_LDFLAGS) -lplatform -lm))
$(eval $(call C_TEST_RULE,test-touch-scroll-routing,tests/test_touch_scroll_routing.c,$(BINDIR)/test_touch_scroll_routing,unite,,$(PLUGINLIBDIR)/UIKit.so $(TEST_LDFLAGS)))
$(eval $(call C_TEST_RULE,test-timer-cancel,tests/test_timer_cancel.c,$(BINDIR)/test_timer_cancel,unite,,$(PLUGINLIBDIR)/UIKit.so $(TEST_LDFLAGS) -lplatform))

HEADLESS_LUA_TESTS = test-listbox test-layout test-state-manager test-animations test-timers test-styles-lua test-body test-console-view test-object-retention test-async test-widget test-router test-application test-url-for test-geometry test-parsers test-object-hierarchy test-tabbar test-tab-interaction test-metro test-text-layout test-stack-layout test-grid-layout test-grid-tracks test-interaction test-adventure-header-hit test-node test-pagehost-listbox
HEADLESS_LUA_TESTS += test-imageview
HEADLESS_LUA_TESTS += test-text-leading
HEADLESS_LUA_TESTS += test-node2d-scroll test-scroll-layout
HEADLESS_LUA_TESTS += test-animation-clock
HEADLESS_LUA_TESTS += test-screen-paint-lifecycle test-activity-indicator test-touch-scroll-routing test-timer-cancel
$(eval $(call LUA_TEST_RULE,test-activity-indicator,tests/test_activity_indicator.lua,unite))

HEADLESS_LUA_TESTS += test-credentials
HEADLESS_LUA_TESTS += test-notifications
HEADLESS_LUA_TESTS += test-chat-polling
HEADLESS_LUA_TESTS += test-string-bindings test-string-properties
HEADLESS_LUA_TESTS += test-two-way-bindings
HEADLESS_LUA_TESTS += test-data-collection
HEADLESS_LUA_TESTS += test-collection-observers test-message-payload
HEADLESS_LUA_TESTS += test-itemscontrol
HEADLESS_LUA_TESTS += test-event-args
.PHONY: test-event-args
HEADLESS_LUA_TESTS += test-event-serialization test-controllers
.PHONY: test-itemscontrol
HEADLESS_LUA_TESTS += test-mvvm-form
HEADLESS_LUA_TESTS += test-scene-format
# Tests in tests/ run in both scene formats. Tests that need one format's loader
# or fixtures live in tests/<format>/ and run only in that build; tests/lisp/
# test_metro_lisp.lua becomes test-metro-lisp.
FORMAT_LUA_TESTS := $(patsubst tests/$(SCENE_FORMAT)/test_%.lua,test-%,$(wildcard tests/$(SCENE_FORMAT)/test_*.lua))
FORMAT_LUA_TESTS := $(subst _,-,$(FORMAT_LUA_TESTS))
$(foreach t,$(FORMAT_LUA_TESTS),$(eval $(call LUA_TEST_RULE,$(t),tests/$(SCENE_FORMAT)/$(subst -,_,$(t)).lua,unite)))
.PHONY: $(FORMAT_LUA_TESTS)
HEADLESS_LUA_TESTS += $(FORMAT_LUA_TESTS)
.PHONY: test-scene-format
test-scene-format: unite
	$(Q)ORCA_EXPECT_SCENE_FORMAT=$(SCENE_FORMAT) $(TARGET) -test=tests/test_scene_format.lua
HEADLESS_LUA_TESTS += test-footer test-safe-area test-viewport-observer test-avatar-loader test-chat-photo-loader

$(eval $(call LUA_TEST_RULE,test-viewport-observer,tests/test_viewport_observer.lua,unite))
$(eval $(call LUA_TEST_RULE,test-avatar-loader,tests/test_avatar_loader.lua,unite))
$(eval $(call LUA_TEST_RULE,test-chat-photo-loader,tests/test_chat_photo_loader.lua,unite))

$(eval $(call LUA_TEST_RULE,test-footer,tests/test_footer.lua,unite))
$(eval $(call LUA_TEST_RULE,test-safe-area,tests/test_safe_area.lua,unite))


$(eval $(call LUA_TEST_RULE,test-two-way-bindings,tests/test_two_way_bindings.lua,unite))
$(eval $(call LUA_TEST_RULE,test-data-collection,tests/test_data_collection.lua,unite))
$(eval $(call LUA_TEST_RULE,test-mvvm-form,tests/test_mvvm_form_sample.lua,unite))

$(eval $(call LUA_TEST_RULE,test-string-bindings,tests/test_string_bindings.lua,unite))

.PHONY: test-chat-polling
test-chat-polling: unite
	$(TARGET) -test=tests/test_chat_polling.lua
	$(TARGET) -test=tests/test_chat_sync.lua
	$(TARGET) -test=tests/test_chat_refresh_race.lua
	$(TARGET) -test=tests/test_chat_timeline.lua
	$(TARGET) -test=tests/test_chat_diagnostics.lua
	$(TARGET) -test=tests/test_chat_app.lua
	$(TARGET) -test=tests/test_chat_loading.lua

$(eval $(call LUA_TEST_RULE,test-credentials,tests/test_credentials.lua,unite))
.PHONY: test-notifications
$(eval $(call LUA_TEST_RULE,test-notifications,tests/test_notifications.lua,unite))
ifeq ($(UNAME_S),Darwin)
.PHONY: test-notifications-native
$(eval $(call C_TEST_RULE,test-notifications-native,tests/test_notifications.m,$(BINDIR)/test_notifications,unite,,$(TEST_LDFLAGS) -framework Foundation -framework UserNotifications))
test-headless: test-notifications-native
endif
.PHONY: test-imageview
$(eval $(call LUA_TEST_RULE,test-imageview,tests/test_imageview.lua,unite))
.PHONY: test-screen-paint-lifecycle
$(eval $(call LUA_TEST_RULE,test-screen-paint-lifecycle,tests/test_screen_paint_lifecycle.lua,unite))

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
$(eval $(call LUA_TEST_RULE,test-metro,tests/test_metro.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-text-layout,tests/test_text_layout.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-node2d-scroll,tests/test_node2d_scroll.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-stack-layout,tests/test_stack_layout.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-grid-layout,tests/test_grid_layout.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-grid-tracks,tests/test_grid_tracks.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-interaction,tests/test_interaction.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-adventure-header-hit,tests/test_adventure_header_hit.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-node,tests/test_node.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-pagehost-listbox,tests/test_pagehost_listbox.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-listbox,tests/test_listbox.lua,app copyshare))
$(eval $(call LUA_TEST_RULE,test-itemscontrol,tests/test_itemscontrol.lua,unite))
$(eval $(call LUA_TEST_RULE,test-event-args,tests/test_event_args.lua,unite))
$(eval $(call LUA_TEST_RULE,test-event-serialization,tests/test_event_serialization.lua,unite))
$(eval $(call LUA_TEST_RULE,test-controllers,tests/test_controllers.lua,unite))

test-headless: unite test-properties test-styles test-filesystem test-message-registry test-trigger-actions test-editor test-shader-uniforms test-texture-downscale $(HEADLESS_LUA_TESTS) $(HEADLESS_IOS_TESTS)

test: test-headless test-text-layout test-stack-layout test-grid-layout test-grid-tracks test-interaction test-tab-interaction test-node test-node2d-scroll
	$(Q)echo "Running test"
	$(Q)$(TARGET) -test=tests/test$(SCENE_FORMAT_EXT)

include Makefile.webgl

# Apply selection after all ordinary flags, including command-line overrides.
override CFLAGS += $(CPPFLAGS) $(SCENE_FORMAT_FLAGS)
