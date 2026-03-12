MODULES := \
    core/core \
    renderer/api/renderer \
    geometry/geometry \
		filesystem/filesystem \
    UIKit/UIKit \
    SceneKit/SceneKit \
    SpriteKit/SpriteKit

UI_MODULES := \
    UIKit/UIKit \
    SceneKit/SceneKit \
    SpriteKit/SpriteKit

DOC_MODULES := \
    geometry/geometry \
    core/core \
		filesystem/filesystem \
    UIKit/UIKit \
    SceneKit/SceneKit \
    SpriteKit/SpriteKit

SOURCE_DIR := ../source
TEMPLATES  := templates
DOCS_DIR   := ../docs

PYPHP := python3 -m pyphp.pyphp

# Top-level targets
.PHONY: all dtd docs docs-index $(MODULES)

all: $(MODULES)

# Per-module rule — each module builds its 3 files
define MODULE_RULE
$(1):
	@echo "Generating $(1)..."
	$(PYPHP) $(TEMPLATES)/header.php     $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1).h
	$(PYPHP) $(TEMPLATES)/properties.php $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1)_properties.h
	$(PYPHP) $(TEMPLATES)/export.php     $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1)_export.c
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_RULE,$(mod))))

# Generate orca.dtd from UI modules
dtd:
	@echo "Generating DTD..."
	$(PYPHP) $(TEMPLATES)/dtd.php \
		$(SOURCE_DIR)/UIKit/UIKit.xml \
		$(SOURCE_DIR)/SceneKit/SceneKit.xml \
		$(SOURCE_DIR)/SpriteKit/SpriteKit.xml \
		$(SOURCE_DIR)/filesystem/filesystem.xml \
		> $(DOCS_DIR)/schemas/orca.dtd

# Generate Markdown API docs for each documented module
define DOCS_RULE
docs-$(1):
	@echo "Generating docs for $(1)..."
	$(PYPHP) $(TEMPLATES)/docs.php $(SOURCE_DIR)/$(1).xml
endef

$(foreach mod,$(DOC_MODULES),$(eval $(call DOCS_RULE,$(mod))))

docs-index:
	@echo "Generating docs index..."
	$(PYPHP) $(TEMPLATES)/docs_index.php \
		$(foreach mod,$(DOC_MODULES),$(SOURCE_DIR)/$(mod).xml)

docs: $(foreach mod,$(DOC_MODULES),docs-$(mod)) docs-index