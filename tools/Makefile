MODULES := \
    source/core/core \
    source/renderer/api/renderer \
    source/geometry/geometry \
		source/filesystem/filesystem \
    plugins/UIKit/UIKit \
    plugins/SceneKit/SceneKit \
    plugins/SpriteKit/SpriteKit \
    plugins/DarkReign/DarkReign

UI_MODULES := \
    plugins/UIKit/UIKit \
    plugins/SceneKit/SceneKit \
    plugins/SpriteKit/SpriteKit

DOC_MODULES := \
    source/geometry/geometry \
    source/core/core \
		source/filesystem/filesystem \
    plugins/UIKit/UIKit \
    plugins/SceneKit/SceneKit \
    plugins/SpriteKit/SpriteKit

DOCS_DIR   := ../docs
TEMPLATES  := templates

PYPHP := python3 -m pyphp.pyphp

# Top-level targets
.PHONY: all dtd docs docs-index $(MODULES)

all: $(MODULES)

# Per-module rule — each module builds its 3 files
define MODULE_RULE
$(1):
	@echo "Generating $(1)..."
	$(PYPHP) $(TEMPLATES)/header.php     ../$(1).xml > ../$(1).h
	$(PYPHP) $(TEMPLATES)/properties.php ../$(1).xml > ../$(1)_properties.h
	$(PYPHP) $(TEMPLATES)/export.php     ../$(1).xml > ../$(1)_export.c
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_RULE,$(mod))))

# Generate orca.dtd from UI modules
dtd:
	@echo "Generating DTD..."
	$(PYPHP) $(TEMPLATES)/dtd.php \
		../plugins/UIKit/UIKit.xml \
		../plugins/SceneKit/SceneKit.xml \
		../plugins/SpriteKit/SpriteKit.xml \
		../source/filesystem/filesystem.xml \
		> $(DOCS_DIR)/schemas/orca.dtd

# Generate Markdown API docs for each documented module
define DOCS_RULE
docs-$(1):
	@echo "Generating docs for $(1)..."
	$(PYPHP) $(TEMPLATES)/docs.php ../$(1).xml
endef

$(foreach mod,$(DOC_MODULES),$(eval $(call DOCS_RULE,$(mod))))

docs-index:
	@echo "Generating docs index..."
	$(PYPHP) $(TEMPLATES)/docs_index.php \
		$(foreach mod,$(DOC_MODULES),../$(mod).xml)

docs: $(foreach mod,$(DOC_MODULES),docs-$(mod)) docs-index