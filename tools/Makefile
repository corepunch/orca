MODULES := \
    core/core \
    renderer/api/renderer \
    geometry/geometry \
    UIKit/UIKit \
    SceneKit/SceneKit \
    SpriteKit/SpriteKit

SOURCE_DIR := ../source
TEMPLATES  := templates

PYPHP := python3 -m pyphp.pyphp

# Top-level targets
.PHONY: all $(MODULES)

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