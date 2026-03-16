SOURCE_MODULES := \
    core/core \
    renderer/api/renderer \
    geometry/geometry \
		filesystem/filesystem

PLUGIN_MODULES := \
    UIKit/UIKit \
    SceneKit/SceneKit \
    SpriteKit/SpriteKit

MODULES := $(SOURCE_MODULES) $(PLUGIN_MODULES)

DOC_MODULES := \
    geometry/geometry \
    core/core \
		filesystem/filesystem \
    UIKit/UIKit \
    SceneKit/SceneKit \
    SpriteKit/SpriteKit

SOURCE_DIR := ../source
PLUGIN_DIR := ../plugins
TEMPLATES  := templates
DOCS_DIR   := ../docs

PYPHP := python3 -m pyphp.pyphp

# Top-level targets
.PHONY: all dtd docs docs-index $(MODULES)

all: $(MODULES)

# Per-module rule for source modules — each module builds its 3 files
define MODULE_RULE
$(1):
	@echo "Generating $(1)..."
	$(PYPHP) $(TEMPLATES)/header.php     $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1).h
	$(PYPHP) $(TEMPLATES)/properties.php $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1)_properties.h
	$(PYPHP) $(TEMPLATES)/export.php     $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1)_export.c
endef

$(foreach mod,$(SOURCE_MODULES),$(eval $(call MODULE_RULE,$(mod))))

# Per-module rule for plugin modules — each module builds its 3 files into plugins/
define PLUGIN_RULE
$(1):
	@echo "Generating $(1)..."
	$(PYPHP) $(TEMPLATES)/header.php     $(PLUGIN_DIR)/$(1).xml > $(PLUGIN_DIR)/$(1).h
	$(PYPHP) $(TEMPLATES)/properties.php $(PLUGIN_DIR)/$(1).xml > $(PLUGIN_DIR)/$(1)_properties.h
	$(PYPHP) $(TEMPLATES)/export.php     $(PLUGIN_DIR)/$(1).xml > $(PLUGIN_DIR)/$(1)_export.c
endef

$(foreach mod,$(PLUGIN_MODULES),$(eval $(call PLUGIN_RULE,$(mod))))

# Generate orca.dtd from UI modules
dtd:
	@echo "Generating DTD..."
	$(PYPHP) $(TEMPLATES)/dtd.php \
		$(PLUGIN_DIR)/UIKit/UIKit.xml \
		$(PLUGIN_DIR)/SceneKit/SceneKit.xml \
		$(PLUGIN_DIR)/SpriteKit/SpriteKit.xml \
		$(SOURCE_DIR)/filesystem/filesystem.xml \
		> $(DOCS_DIR)/schemas/orca.dtd

# Generate Markdown API docs for each documented module
define SOURCE_DOCS_RULE
docs-$(1):
	@echo "Generating docs for $(1)..."
	$(PYPHP) $(TEMPLATES)/docs.php $(SOURCE_DIR)/$(1).xml
endef

define PLUGIN_DOCS_RULE
docs-$(1):
	@echo "Generating docs for $(1)..."
	$(PYPHP) $(TEMPLATES)/docs.php $(PLUGIN_DIR)/$(1).xml
endef

$(foreach mod,$(SOURCE_MODULES),$(eval $(call SOURCE_DOCS_RULE,$(mod))))
$(foreach mod,$(PLUGIN_MODULES),$(eval $(call PLUGIN_DOCS_RULE,$(mod))))

docs-index:
	@echo "Generating docs index..."
	$(PYPHP) $(TEMPLATES)/docs_index.php \
		$(foreach mod,geometry/geometry core/core filesystem/filesystem,$(SOURCE_DIR)/$(mod).xml) \
		$(foreach mod,$(PLUGIN_MODULES),$(PLUGIN_DIR)/$(mod).xml)

docs: $(foreach mod,$(DOC_MODULES),docs-$(mod)) docs-index