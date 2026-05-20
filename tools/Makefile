MODULE_ROOT ?= ../
MODULES := $(patsubst $(MODULE_ROOT)%.cgen,%,$(shell find $(MODULE_ROOT)source $(MODULE_ROOT)plugins -name '*.cgen' -type f | sort))
CGEN := $(MODULE_ROOT)build/bin/cgen
CG_PLUGIN_HEADER := $(MODULE_ROOT)build/plugins/codegen/libheader.so
CG_PLUGIN_PROPERTIES := $(MODULE_ROOT)build/plugins/codegen/libproperties.so
CG_PLUGIN_EXPORT := $(MODULE_ROOT)build/plugins/codegen/libexport.so
CG_PREVIEW_DIR := $(MODULE_ROOT)build/codegen/out

# Top-level targets
.PHONY: all codegen-host codegen-sample modules-c-preview modules-c-diff $(MODULES)

all: $(MODULES)

# Per-module rule — each module builds its 3 files
define MODULE_RULE
$(1): codegen-host
	@echo "Generating $(1)..."
	$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_HEADER) > $(MODULE_ROOT)$(1).h
	$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_PROPERTIES) > $(MODULE_ROOT)$(1)_properties.h
	$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_EXPORT) > $(MODULE_ROOT)$(1)_export.c
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_RULE,$(mod))))

define MODULE_C_PREVIEW_RULE
cgen-preview-$(1): codegen-host
	@echo "Preview C codegen $(1)..."
	@mkdir -p $(CG_PREVIEW_DIR)/$(dir $(1))
	$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_HEADER) > $(CG_PREVIEW_DIR)/$(1).h
	$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_PROPERTIES) > $(CG_PREVIEW_DIR)/$(1)_properties.h
	$(CGEN) $(MODULE_ROOT)$(1).cgen $(CG_PLUGIN_EXPORT) > $(CG_PREVIEW_DIR)/$(1)_export.c
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_C_PREVIEW_RULE,$(mod))))

modules-c-preview: $(foreach mod,$(MODULES),cgen-preview-$(mod))

define MODULE_C_DIFF_RULE
cgen-diff-$(1):
	@echo "Diff C preview $(1)..."
	@cmp -s $(MODULE_ROOT)$(1).h $(CG_PREVIEW_DIR)/$(1).h || echo "  != $(1).h"
	@cmp -s $(MODULE_ROOT)$(1)_properties.h $(CG_PREVIEW_DIR)/$(1)_properties.h || echo "  != $(1)_properties.h"
	@cmp -s $(MODULE_ROOT)$(1)_export.c $(CG_PREVIEW_DIR)/$(1)_export.c || echo "  != $(1)_export.c"
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_C_DIFF_RULE,$(mod))))

modules-c-diff: modules-c-preview $(foreach mod,$(MODULES),cgen-diff-$(mod))

codegen-host:
	$(MAKE) -C $(MODULE_ROOT). codegen-host

codegen-sample: codegen-host
	$(MAKE) -C $(MODULE_ROOT). codegen-sample
