# GLSL to MSL translation for the Metal renderer: glslang and SPIRV-Cross,
# compiled from their submodules. Includers set SHADER_GEN, the directory
# that receives glslang's generated build_info.h, and compile SHADER_SOURCES
# with SHADER_CXXFLAGS.
GLSLANG_DIR := libs/glslang
SPIRV_CROSS_DIR := libs/SPIRV-Cross
GLSLANG_SOURCES := $(wildcard $(GLSLANG_DIR)/glslang/MachineIndependent/*.cpp) \
	$(wildcard $(GLSLANG_DIR)/glslang/MachineIndependent/preprocessor/*.cpp) \
	$(GLSLANG_DIR)/glslang/GenericCodeGen/CodeGen.cpp $(GLSLANG_DIR)/glslang/GenericCodeGen/Link.cpp \
	$(GLSLANG_DIR)/glslang/OSDependent/Unix/ossource.cpp \
	$(GLSLANG_DIR)/glslang/ResourceLimits/ResourceLimits.cpp \
	$(addprefix $(GLSLANG_DIR)/SPIRV/,GlslangToSpv.cpp InReadableOrder.cpp Logger.cpp SpvBuilder.cpp SpvPostProcess.cpp SpvTools.cpp)
SPIRV_CROSS_SOURCES := $(addprefix $(SPIRV_CROSS_DIR)/,spirv_cross.cpp spirv_parser.cpp spirv_cross_parsed_ir.cpp spirv_cfg.cpp spirv_glsl.cpp spirv_msl.cpp)
SHADER_SOURCES := $(GLSLANG_SOURCES) $(SPIRV_CROSS_SOURCES) source/renderer/metal/glsl_msl.cpp
SHADER_INCLUDES := -I$(SHADER_GEN) -I$(GLSLANG_DIR) -I$(SPIRV_CROSS_DIR)
SHADER_CXXFLAGS := -std=c++17 -w $(SHADER_INCLUDES)
GLSLANG_VERSION := 15 4 0

$(SHADER_GEN)/glslang/build_info.h: $(GLSLANG_DIR)/build_info.h.tmpl tools/shaders/shaders.mk
	@mkdir -p "$(@D)"
	set -- $(GLSLANG_VERSION); sed -e "s/@major@/$$1/" -e "s/@minor@/$$2/" -e "s/@patch@/$$3/" -e "s/@flavor@//" "$<" > "$@"

# The build configuration for tools/ios/dependencies.py, which compiles the
# same sources into libglslmsl.a for the iOS and Xcode builds.
.PHONY: shader-config
shader-config:
	@echo $(GLSLANG_VERSION)
	@echo $(SHADER_SOURCES)
