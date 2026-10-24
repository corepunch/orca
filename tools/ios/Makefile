.DEFAULT_GOAL := app
.DELETE_ON_ERROR:
DEVELOPER_DIR ?= $(shell xcode-select -p)
export DEVELOPER_DIR
SDK ?= iphoneos
ARCH ?= $(if $(filter iphoneos,$(SDK)),arm64,$(shell uname -m))
ifeq ($(filter $(SDK),iphoneos iphonesimulator),)
$(error SDK must be iphoneos or iphonesimulator)
endif
ifeq ($(filter $(ARCH),arm64 x86_64),)
$(error ARCH must be arm64 or x86_64)
endif
ifeq ($(SDK)-$(ARCH),iphoneos-x86_64)
$(error Physical iPhone/iPad builds require ARCH=arm64)
endif
IOS_MIN ?= 16.0
PROJECT ?= apps/chat
BUNDLE_ID ?= org.orca.chat
DISPLAY_NAME ?= ORCA Chat
DEVICE ?=
DEVICE_TYPE ?= iPhone
TEAM ?=
PROFILE ?=
BUILD_DIR ?= build/ios
ROOT := $(abspath $(BUILD_DIR))/$(SDK)-$(ARCH)
DEPS := $(ROOT)/deps
DEPS_STAMP := $(DEPS)/settings.json
BUNDLE := $(ROOT)/Orca.app
SDK_PATH := $(shell xcrun --sdk $(SDK) --show-sdk-path)
CC := xcrun --sdk $(SDK) clang
MIN_FLAG := $(if $(filter iphoneos,$(SDK)),-miphoneos-version-min,-mios-simulator-version-min)=$(IOS_MIN)
FLAGS := -arch $(ARCH) -isysroot "$(SDK_PATH)" $(MIN_FLAG) -O2 -g -MMD -MP -DORCA_IOS=1 -DORCA_STATIC=1 -DGL_SILENCE_DEPRECATION -Wno-deprecated-declarations -I. -I$(ROOT)/include -I$(DEPS)/include -I$(DEPS)/include/freetype2 -I"$(SDK_PATH)/usr/include/libxml2"
CFLAGS := $(FLAGS) -Igenerated
# UIKit/UIKit.h in generated/ is the engine API, not Apple's framework header.
OBJCFLAGS := $(FLAGS) -idirafter generated -fmodules-cache-path=$(ROOT)/module-cache
MODULES := geometry orca sysutil console parsers debug network renderer filesystem core data
PLUGINS := UIKit SceneKit SpriteKit
OBJECTS := $(addprefix $(ROOT)/obj/,$(addsuffix .o,$(MODULES))) $(addprefix $(ROOT)/obj/plugin_,$(addsuffix .o,$(PLUGINS)))
PLATFORM_SOURCES := $(wildcard libs/platform/ios/*.m) $(wildcard libs/platform/unix/*.c)
PLATFORM_OBJECTS := $(addprefix $(ROOT)/obj/,$(addsuffix .o,$(basename $(PLATFORM_SOURCES))))
LIBS := $(addprefix $(DEPS)/lib/,liblua.a libfreetype.a liblz4.a libcurl.a) -lxml2 -lz -liconv -lm -framework UIKit -framework Foundation -framework OpenGLES -framework QuartzCore -framework CoreGraphics -framework CoreText -framework Security -framework SystemConfiguration -framework UniformTypeIdentifiers -framework AudioToolbox
.PHONY: app dependencies generated settings run install list-devices
generated:
	$(MAKE) ARCH=$(shell uname -m) modules
dependencies:
	python3 tools/ios/dependencies.py --sdk $(SDK) --arch $(ARCH) --minimum $(IOS_MIN) --build-dir "$(BUILD_DIR)"
$(DEPS_STAMP): dependencies
settings:
	@mkdir -p "$(ROOT)/include" "$(ROOT)/obj"
	@printf '%s\n' '$(CC) $(CFLAGS) $(OBJCFLAGS) $(LIBS)' > "$(ROOT)/settings.tmp"
	@cmp -s "$(ROOT)/settings.tmp" "$(ROOT)/settings" || cp "$(ROOT)/settings.tmp" "$(ROOT)/settings"
	@printf '%s\n' 'extern int luaopen_orca_UIKit(lua_State *);' 'extern int luaopen_orca_SceneKit(lua_State *);' 'extern int luaopen_orca_SpriteKit(lua_State *);' 'static const luaL_Reg plugin_modules[] = {' '{"orca.UIKit", luaopen_orca_UIKit},' '{"orca.SceneKit", luaopen_orca_SceneKit},' '{"orca.SpriteKit", luaopen_orca_SpriteKit},' '{NULL, NULL}};' > "$(ROOT)/include/plugins_luaopen.h.tmp"
	@cmp -s "$(ROOT)/include/plugins_luaopen.h.tmp" "$(ROOT)/include/plugins_luaopen.h" || cp "$(ROOT)/include/plugins_luaopen.h.tmp" "$(ROOT)/include/plugins_luaopen.h"
$(ROOT)/settings: settings

define unity
$(ROOT)/obj/$(1).o: $$(shell find $(2) -name '*.c' 2>/dev/null) $(ROOT)/settings tools/ios/Makefile | generated dependencies
	@mkdir -p "$$(@D)"
	@find $(2) -name '*.c' | sort | sed 's/.*/#include "&"/' > "$(ROOT)/obj/$(1).c"
	$(CC) $(CFLAGS) -c "$(ROOT)/obj/$(1).c" -o "$$@"
endef
$(foreach module,$(MODULES),$(eval $(call unity,$(module),source/$(module))))
$(foreach plugin,$(PLUGINS),$(eval $(call unity,plugin_$(plugin),plugins/$(plugin))))
$(ROOT)/obj/%.o: %.m $(ROOT)/settings tools/ios/Makefile | dependencies generated
	@mkdir -p "$(@D)"
	$(CC) $(OBJCFLAGS) -c "$<" -o "$@"
$(ROOT)/obj/%.o: %.c $(ROOT)/settings tools/ios/Makefile | dependencies generated
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c "$<" -o "$@"
$(ROOT)/Orca: $(OBJECTS) $(PLATFORM_OBJECTS) $(ROOT)/obj/source/ios.o $(DEPS_STAMP)
	$(CC) -arch $(ARCH) -isysroot "$(SDK_PATH)" $(MIN_FLAG) $(OBJECTS) $(PLATFORM_OBJECTS) $(ROOT)/obj/source/ios.o $(LIBS) -o "$@"
app: $(ROOT)/Orca
	python3 tools/ios/bundle.py --binary "$<" --bundle "$(BUNDLE)" --project "$(PROJECT)" --sdk $(SDK) --identifier "$(BUNDLE_ID)" --minimum $(IOS_MIN) --display-name "$(DISPLAY_NAME)"
ifeq ($(SDK),iphonesimulator)
	codesign --force --sign - "$(BUNDLE)"
endif
run: app
	@test "$(SDK)" = iphonesimulator || { echo 'Use SDK=iphonesimulator for run'; exit 1; }
	python3 tools/ios/deploy.py --simulator --bundle "$(BUNDLE)" --device-type "$(DEVICE_TYPE)" $(if $(DEVICE),--device "$(DEVICE)")
install: app
	@test "$(SDK)" = iphoneos -a "$(ARCH)" = arm64 || { echo 'Use SDK=iphoneos ARCH=arm64 for install'; exit 1; }
	python3 tools/ios/deploy.py --bundle "$(BUNDLE)" --device-type "$(DEVICE_TYPE)" $(if $(DEVICE),--device "$(DEVICE)") $(if $(TEAM),--team "$(TEAM)") $(if $(PROFILE),--profile "$(PROFILE)")
list-devices:
	xcrun devicectl list devices
	xcrun simctl list devices available
-include $(OBJECTS:.o=.d) $(PLATFORM_OBJECTS:.o=.d) $(ROOT)/obj/source/ios.d
