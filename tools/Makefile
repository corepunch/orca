MODULES := \
    core/core \
    renderer/api/renderer \
    geometry/geometry \
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
	$(PYPHP) $(TEMPLATES)/index.php codegen/header     $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1).h
	$(PYPHP) $(TEMPLATES)/index.php codegen/properties $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1)_properties.h
	$(PYPHP) $(TEMPLATES)/index.php codegen/export     $(SOURCE_DIR)/$(1).xml > $(SOURCE_DIR)/$(1)_export.c
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_RULE,$(mod))))

# Generate orca.dtd from UI modules
dtd:
	@echo "Generating DTD..."
	$(PYPHP) $(TEMPLATES)/index.php dtd/generate \
		$(SOURCE_DIR)/UIKit/UIKit.xml \
		$(SOURCE_DIR)/SceneKit/SceneKit.xml \
		$(SOURCE_DIR)/SpriteKit/SpriteKit.xml \
		> $(DOCS_DIR)/schemas/orca.dtd

# Generate Markdown API docs for each documented module
define DOCS_RULE
docs-$(1):
	@echo "Generating docs for $(1)..."
	$(PYPHP) $(TEMPLATES)/index.php docs/generate $(SOURCE_DIR)/$(1).xml
endef

$(foreach mod,$(DOC_MODULES),$(eval $(call DOCS_RULE,$(mod))))

docs-index:
	@echo "Generating docs index..."
	$(PYPHP) $(TEMPLATES)/index.php docs/index \
		$(foreach mod,$(DOC_MODULES),$(SOURCE_DIR)/$(mod).xml)

docs: $(foreach mod,$(DOC_MODULES),docs-$(mod)) docs-index