OUTDIR ?= .
LIBNAME = libplatform.$(LIB_EXT)
TARGET = $(OUTDIR)/$(LIBNAME)
UNAME_S := $(shell uname -s)
HASH := \#

TEST_SRC = /tmp/test_platform_api.c
TEST_BIN = /tmp/test_platform_api
TEST_MSG_BIN  = /tmp/test_messages
TEST_TIMER_BIN = /tmp/test_timer

ifdef EMSCRIPTEN
	CC = emcc
	CFLAGS = -Wall -Wextra -fPIC -I.
	LDFLAGS = -sSIDE_MODULE=1 -sUSE_WEBGL2=1 -sMIN_WEBGL_VERSION=1 -sMAX_WEBGL_VERSION=2
	LIB_EXT = wasm
	FIND_SOURCES = find webgl -name "*.c"
	LANG = c
else ifeq ($(UNAME_S),Darwin)
	CC = clang
	CFLAGS = -Wall -Wextra -fPIC -I. -DGL_SILENCE_DEPRECATION
	LDFLAGS = -dynamiclib -framework AppKit -framework Cocoa -framework OpenGL -framework IOSurface -install_name @rpath/$(LIBNAME)
	LIB_EXT = dylib
	FIND_SOURCES = ( find macos -name "*.m"; find unix -name "*.c"; )
	LANG = objective-c
	TEST_LDFLAGS = -L$(abspath $(OUTDIR)) -lplatform -rpath $(abspath $(OUTDIR))
else ifeq ($(UNAME_S),Linux)
	CC = gcc
	CFLAGS = -Wall -Wextra -fPIC -I.
	LDFLAGS = -shared
	LIB_EXT = so
	# Try to detect Wayland libraries
	WAYLAND_LIBS := $(shell pkg-config --libs wayland-client wayland-egl xkbcommon egl gl 2>/dev/null)
	ifneq ($(WAYLAND_LIBS),)
		CFLAGS += $(shell pkg-config --cflags wayland-client wayland-egl xkbcommon egl gl 2>/dev/null)
		LDFLAGS += $(WAYLAND_LIBS)
		FIND_SOURCES = ( find wayland -name "*.c"; find unix -name "*.c"; )
	else
		# Try to detect X11 libraries as fallback
		X11_LIBS := $(shell pkg-config --libs x11 egl gl 2>/dev/null)
		ifneq ($(X11_LIBS),)
			CFLAGS += $(shell pkg-config --cflags x11 egl gl 2>/dev/null)
			LDFLAGS += $(X11_LIBS)
			FIND_SOURCES = ( find x11 -name "*.c"; find unix -name "*.c"; )
		else
			# Fallback to unix-only (no windowing support)
			FIND_SOURCES = find unix -name "*.c"
		endif
	endif
	LANG = c
	TEST_LDFLAGS = -L$(abspath $(OUTDIR)) -lplatform -Wl,-rpath,$(abspath $(OUTDIR))
else ifneq (,$(findstring MINGW,$(UNAME_S))$(findstring MSYS,$(UNAME_S)))
	CC = gcc
	CFLAGS = -Wall -Wextra -I. -DPLATFORM_BUILD
	LDFLAGS = -shared \
	          -Wl,--out-implib,$(OUTDIR)/libplatform.dll.a \
	          -lopengl32 -lgdi32 -luser32 -lcomdlg32 \
	          -lole32 -lshell32 -ladvapi32 -lws2_32
	LIB_EXT = dll
	FIND_SOURCES = find windows -name "*.c"
	LANG = c
	# Put the test binary next to the DLL so Windows finds it at runtime
	TEST_SRC = $(OUTDIR)/test_platform_api_tmp.c
	TEST_BIN = $(OUTDIR)/test_platform_api.exe
	TEST_LDFLAGS = -L$(abspath $(OUTDIR)) -lplatform
	TEST_MSG_BIN   = $(OUTDIR)/test_messages.exe
	TEST_TIMER_BIN = $(OUTDIR)/test_timer.exe
else
	$(error Unsupported OS: $(UNAME_S))
endif

all: $(TARGET)

$(TARGET):
	$(FIND_SOURCES) | sed 's|.*|$(HASH)include "&"|' | $(CC) $(CFLAGS) -x $(LANG) - $(LDFLAGS) -o $@

# Parse platform.h to find all WI_API functions, generate a C test that asserts
# each function pointer is non-NULL (i.e. the symbol is defined), then compile,
# link and run that test against the built library.
ifdef EMSCRIPTEN
test:
	@echo "API test skipped for WebGL/Emscripten (WASM side modules cannot be linked natively)"
else
test: $(TARGET)
	@printf '#include "platform.h"\n#include <assert.h>\nint main(void) {\n' > $(TEST_SRC)
	@awk '/^WI_API[[:space:]]/{getline; sub(/[(].*/,""); printf "    assert(%s != NULL);\n", $$1}' platform.h >> $(TEST_SRC)
	@printf '    return 0;\n}\n' >> $(TEST_SRC)
	@$(CC) -I. $(TEST_SRC) $(TEST_LDFLAGS) -o $(TEST_BIN)
	@$(TEST_BIN)
	@rm -f $(TEST_SRC) $(TEST_BIN)
	@echo "All platform API functions are defined."
	@$(CC) -I. tests/test_messages.c $(TEST_LDFLAGS) -o $(TEST_MSG_BIN)
	@$(TEST_MSG_BIN)
	@rm -f $(TEST_MSG_BIN)
	@echo "Message queue tests passed."
	@$(CC) -I. tests/test_timer.c $(TEST_LDFLAGS) -o $(TEST_TIMER_BIN)
	@$(TEST_TIMER_BIN)
	@rm -f $(TEST_TIMER_BIN)
	@echo "Timer tests passed."
endif

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -d /usr/local/lib /usr/local/include
	install -m 755 $(TARGET) /usr/local/lib/
	install -m 644 platform.h /usr/local/include/

.PHONY: all clean install test
