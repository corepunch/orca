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

PYPHP   := python3 -m pyphp.pyphp
CODEGEN := $(CURDIR)/codegen/codegen

# Build the C codegen tool first
$(CODEGEN):
	$(MAKE) -C codegen

# Top-level targets
.PHONY: all dtd docs docs-index $(MODULES)

all: $(CODEGEN) $(MODULES)

# Per-module rule — each module builds its 3 files.
# Uses the compiled C codegen tool (with .ctmpl templates) when available,
# falling back to the Python/pyphp pipeline for export.c (not yet ported).
define MODULE_RULE
$(1): $(CODEGEN)
	@echo "Generating $(1)..."
	$(CODEGEN) ../$(1).xml ../$(1).h         $(TEMPLATES)/header.ctmpl
	$(CODEGEN) ../$(1).xml ../$(1)_properties.h $(TEMPLATES)/properties.ctmpl
	$(PYPHP)   $(TEMPLATES)/export.php       ../$(1).xml > ../$(1)_export.c
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