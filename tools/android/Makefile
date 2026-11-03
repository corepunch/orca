# Android: one native library (libmain.so) per ABI, linked statically like the
# iOS app, then packaged with Gradle. Run from the repository root:
#   make -f tools/android/Makefile PROJECT=apps/chat APPLICATION_ID=org.orca.chat apk
.DEFAULT_GOAL := apk
.DELETE_ON_ERROR:
ANDROID_NDK ?= $(or $(ANDROID_NDK_HOME),$(ANDROID_NDK_LATEST_HOME))
ABI ?= arm64-v8a
ABIS ?= $(ABI)
API ?= 26
PROJECT ?= apps/chat
APPLICATION_ID ?= org.orca.chat
DISPLAY_NAME ?= ORCA Chat
BUILD_DIR ?= build/android
ifeq ($(ANDROID_NDK),)
$(error Set ANDROID_NDK_HOME to an Android NDK)
endif
ifeq ($(filter $(ABI),arm64-v8a x86_64),)
$(error ABI must be arm64-v8a or x86_64)
endif
TRIPLE := $(if $(filter arm64-v8a,$(ABI)),aarch64,x86_64)-linux-android
HOST := $(firstword $(wildcard $(ANDROID_NDK)/toolchains/llvm/prebuilt/*))
CC := $(HOST)/bin/clang --target=$(TRIPLE)$(API)
ROOT := $(abspath $(BUILD_DIR))/$(ABI)
DEPS := $(ROOT)/deps
GLUE := $(ANDROID_NDK)/sources/android/native_app_glue
FLAGS := -fPIC -O2 -g -MMD -MP -DORCA_STATIC=1 -DHAVE_WINDOWING -I. -I$(ROOT)/include -I$(DEPS)/include \
	-I$(DEPS)/include/libxml2 -Ilibs/lua -Ilibs/lz4/lib -I$(GLUE)
CFLAGS := $(FLAGS) -Igenerated
MODULES := geometry orca sysutil console parsers debug network renderer filesystem core data media credentials notifications signin
PLUGINS := UIKit SceneKit SpriteKit
OBJECTS := $(addprefix $(ROOT)/obj/,$(addsuffix .o,$(MODULES))) $(addprefix $(ROOT)/obj/plugin_,$(addsuffix .o,$(PLUGINS)))
LUA_SOURCES := $(filter-out $(addprefix libs/lua/,lua.c luac.c onelua.c ltests.c),$(wildcard libs/lua/*.c))
NATIVE_OBJECTS := $(ROOT)/obj/platform.o $(ROOT)/obj/lua.o $(ROOT)/obj/lz4.o $(ROOT)/obj/glue.o \
	$(ROOT)/obj/source/android/android_main.o
LIBS := $(addprefix $(DEPS)/lib/,libcurl.a libmbedtls.a libmbedx509.a libmbedcrypto.a libxml2.a) \
	-lEGL -lGLESv3 -landroid -llog -lz -lm -ldl
LIBRARY := $(BUILD_DIR)/gradle/jniLibs/$(ABI)/libmain.so
.PHONY: library dependencies generated settings apk assets clean

generated:
	$(MAKE) modules
dependencies:
	python3 tools/android/dependencies.py --ndk "$(ANDROID_NDK)" --abi $(ABI) --api $(API) --build-dir "$(BUILD_DIR)"
settings:
	@mkdir -p "$(ROOT)/include" "$(ROOT)/obj"
	@printf '%s\n' '$(CC) $(CFLAGS) $(LIBS)' > "$(ROOT)/settings.tmp"
	@cmp -s "$(ROOT)/settings.tmp" "$(ROOT)/settings" || cp "$(ROOT)/settings.tmp" "$(ROOT)/settings"
	@printf '%s\n' 'extern int luaopen_orca_UIKit(lua_State *);' 'extern int luaopen_orca_SceneKit(lua_State *);' 'extern int luaopen_orca_SpriteKit(lua_State *);' 'static const luaL_Reg plugin_modules[] = {' '{"orca.UIKit", luaopen_orca_UIKit},' '{"orca.SceneKit", luaopen_orca_SceneKit},' '{"orca.SpriteKit", luaopen_orca_SpriteKit},' '{NULL, NULL}};' > "$(ROOT)/include/plugins_luaopen.h.tmp"
	@cmp -s "$(ROOT)/include/plugins_luaopen.h.tmp" "$(ROOT)/include/plugins_luaopen.h" || cp "$(ROOT)/include/plugins_luaopen.h.tmp" "$(ROOT)/include/plugins_luaopen.h"
$(ROOT)/settings: settings

# One translation unit per engine module or plugin, as for iOS.
define unity
$(ROOT)/obj/$(1).o: $$(shell find $(2) -name '*.c' 2>/dev/null) $(ROOT)/settings tools/android/Makefile | generated dependencies
	@mkdir -p "$$(@D)"
	@find $(2) -name '*.c' | sort | sed 's/.*/#include "&"/' > "$(ROOT)/obj/$(1).c"
	$(CC) $(CFLAGS) -c "$(ROOT)/obj/$(1).c" -o "$$@"
endef
$(foreach module,$(MODULES),$(eval $(call unity,$(module),source/$(module))))
$(foreach plugin,$(PLUGINS),$(eval $(call unity,plugin_$(plugin),plugins/$(plugin))))

$(ROOT)/obj/platform.o: $(wildcard libs/platform/android/*.[ch] libs/platform/unix/*.c libs/platform/audio/*.c) $(ROOT)/settings | generated
	@mkdir -p "$(@D)"
	@{ ls libs/platform/android/*.c libs/platform/unix/*.c libs/platform/audio/*.c; } | sed 's/.*/#include "&"/' > "$(ROOT)/obj/platform.c"
	$(CC) $(FLAGS) -Ilibs/platform -c "$(ROOT)/obj/platform.c" -o "$@"
$(ROOT)/obj/lua.o: $(LUA_SOURCES) $(ROOT)/settings
	@mkdir -p "$(@D)"
	@printf '#include "%s"\n' $(LUA_SOURCES) > "$(ROOT)/obj/lua.c"
	$(CC) -fPIC -O2 -g -DLUA_USE_LINUX -DLUA_COMPAT_5_3 -I. -Ilibs/lua -c "$(ROOT)/obj/lua.c" -o "$@"
$(ROOT)/obj/lz4.o: libs/lz4/lib/lz4.c $(ROOT)/settings
	@mkdir -p "$(@D)"
	$(CC) -fPIC -O2 -g -c "$<" -o "$@"
$(ROOT)/obj/glue.o: $(GLUE)/android_native_app_glue.c $(ROOT)/settings
	@mkdir -p "$(@D)"
	$(CC) -fPIC -O2 -g -I$(GLUE) -c "$<" -o "$@"
$(ROOT)/obj/%.o: %.c $(ROOT)/settings tools/android/Makefile | dependencies generated
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c "$<" -o "$@"

# NativeActivity finds ANativeActivity_onCreate in the glue; keep it exported.
$(LIBRARY): $(OBJECTS) $(NATIVE_OBJECTS) | dependencies
	@mkdir -p "$(@D)"
	$(CC) -shared -Wl,-soname,libmain.so -Wl,--no-undefined -Wl,-z,max-page-size=16384 \
		-u ANativeActivity_onCreate $(OBJECTS) $(NATIVE_OBJECTS) $(LIBS) -o "$@"
library: $(LIBRARY)

assets:
	python3 tools/android/package.py --project "$(PROJECT)" --output "$(BUILD_DIR)/gradle" \
		--application-id "$(APPLICATION_ID)" --display-name "$(DISPLAY_NAME)"

# Builds each ABI's library, then tools/android/project/app/build/outputs/apk/debug/app-debug.apk.
apk:
	@for abi in $(ABIS); do $(MAKE) -f tools/android/Makefile ABI=$$abi library || exit 1; done
	$(MAKE) -f tools/android/Makefile assets
	gradle -p tools/android/project --no-daemon -PorcaBuild="$(abspath $(BUILD_DIR))/gradle" \
		-PorcaApplicationId="$(APPLICATION_ID)" assembleDebug

clean:
	rm -rf "$(BUILD_DIR)"

-include $(wildcard $(ROOT)/obj/*.d $(ROOT)/obj/source/android/*.d)
