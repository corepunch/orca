MODULE_ROOT ?= ../
MODULES := $(patsubst $(MODULE_ROOT)%.cgen,%,$(shell find $(MODULE_ROOT)source $(MODULE_ROOT)plugins -name '*.cgen' -type f | sort))
BUILDDIR := $(MODULE_ROOT)build
SOURCEDIR := $(MODULE_ROOT)source
PLUGINDIR := $(MODULE_ROOT)plugins
APPNAME := orca
LIBNAME := orca
BINDIR := $(BUILDDIR)/bin
LIBDIR := $(BUILDDIR)/lib
OBJECTDIR := $(BUILDDIR)/obj
PLATFORM_LIBDIR := $(MODULE_ROOT)libs/platform
TARGETLIB := $(LIBDIR)/lib$(LIBNAME).so
CODEGEN := $(BINDIR)/cgen
CODEGEN_SRC := $(shell find $(MODULE_ROOT)tools/codegen/src -name "*.c" 2>/dev/null | sort)
CODEGEN_PLUGIN_SRC := $(shell find $(MODULE_ROOT)tools/codegen/plugins -name "*.c" 2>/dev/null | sort)
CODEGEN_PLUGINS := $(patsubst $(MODULE_ROOT)tools/codegen/plugins/%.c,$(BUILDDIR)/plugins/codegen/lib%.so,$(CODEGEN_PLUGIN_SRC))
CODEGEN_CFLAGS := -O2 -Wall -Wextra -Werror -std=c11 -fPIC $(ARCH_FLAGS) -I$(MODULE_ROOT)tools/codegen/include -I$(MODULE_ROOT)tools/codegen/src $(shell pkg-config --cflags libxml-2.0)
CODEGEN_LDFLAGS := $(shell pkg-config --libs libxml-2.0) -ldl $(ARCH_FLAGS)
CODEGEN_HEADER_PLUGIN := $(BUILDDIR)/plugins/codegen/libheader.so
CODEGEN_PROPERTIES_PLUGIN := $(BUILDDIR)/plugins/codegen/libproperties.so
CODEGEN_EXPORT_PLUGIN := $(BUILDDIR)/plugins/codegen/libexport.so
CGEN := $(CODEGEN)
CG_PLUGIN_HEADER := $(CODEGEN_HEADER_PLUGIN)
CG_PLUGIN_PROPERTIES := $(CODEGEN_PROPERTIES_PLUGIN)
CG_PLUGIN_EXPORT := $(CODEGEN_EXPORT_PLUGIN)
CG_PREVIEW_DIR := $(MODULE_ROOT)build/codegen/out

ifeq ($(V),1)
Q =
else
Q = @
endif

# Top-level targets
.PHONY: all codegen-host codegen-sample codegen-clean modules-c-preview modules-c-diff $(MODULES)

all: $(MODULES)
codegen-host: $(CODEGEN) $(CODEGEN_PLUGINS)
codegen-sample: codegen-host
	$(Q)mkdir -p $(BUILDDIR)/codegen
	$(Q)$(CODEGEN) $(MODULE_ROOT)source/core/core.cgen $(CODEGEN_HEADER_PLUGIN) > $(BUILDDIR)/codegen/core_stub.h
codegen-clean:
	$(Q)rm -f $(CODEGEN)
	$(Q)rm -rf $(BUILDDIR)/plugins/codegen
	$(Q)rm -rf $(BUILDDIR)/codegen

directories:
	$(Q)mkdir -p $(BINDIR)
	$(Q)mkdir -p $(LIBDIR)
	$(Q)mkdir -p $(OBJECTDIR)
	$(Q)mkdir -p $(BUILDDIR)/plugins/codegen
	$(Q)mkdir -p $(BUILDDIR)/codegen

# Per-module rule — each module builds its 3 files
define MODULE_RULE
$(1): codegen-host
	$(Q)echo "Generating $(1)..."
	$(Q)$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_HEADER) > $(MODULE_ROOT)$(1).h
	$(Q)$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_PROPERTIES) > $(MODULE_ROOT)$(1)_properties.h
	$(Q)$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_EXPORT) > $(MODULE_ROOT)$(1)_export.c
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_RULE,$(mod))))

define MODULE_C_PREVIEW_RULE
cgen-preview-$(1): codegen-host
	$(Q)echo "Preview C codegen $(1)..."
	$(Q)mkdir -p $(CG_PREVIEW_DIR)/$(dir $(1))
	$(Q)$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_HEADER) > $(CG_PREVIEW_DIR)/$(1).h
	$(Q)$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_PROPERTIES) > $(CG_PREVIEW_DIR)/$(1)_properties.h
	$(Q)$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_EXPORT) > $(CG_PREVIEW_DIR)/$(1)_export.c
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_C_PREVIEW_RULE,$(mod))))

modules-c-preview: $(foreach mod,$(MODULES),cgen-preview-$(mod))

define MODULE_C_DIFF_RULE
cgen-diff-$(1):
	$(Q)echo "Diff C preview $(1)..."
	$(Q)cmp -s $(MODULE_ROOT)$(1).h $(CG_PREVIEW_DIR)/$(1).h || echo "  != $(1).h"
	$(Q)cmp -s $(MODULE_ROOT)$(1)_properties.h $(CG_PREVIEW_DIR)/$(1)_properties.h || echo "  != $(1)_properties.h"
	$(Q)cmp -s $(MODULE_ROOT)$(1)_export.c $(CG_PREVIEW_DIR)/$(1)_export.c || echo "  != $(1)_export.c"
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_C_DIFF_RULE,$(mod))))

modules-c-diff: modules-c-preview $(foreach mod,$(MODULES),cgen-diff-$(mod))
$(CODEGEN): $(CODEGEN_SRC) | directories
	$(Q)$(CC) $(CODEGEN_CFLAGS) $^ -o $@ $(CODEGEN_LDFLAGS)

$(BUILDDIR)/plugins/codegen/lib%.so: $(MODULE_ROOT)tools/codegen/plugins/%.c $(MODULE_ROOT)tools/codegen/include/cg_api.h | directories
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CODEGEN_CFLAGS) -shared $< -o $@
