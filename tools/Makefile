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
CODEGEN := ../build/bin/codegen
CODEGEN_EXPORT_CFLAGS := $(shell pkg-config --cflags libxml-2.0)
CODEGEN_EXPORT_LIBS := $(shell pkg-config --libs libxml-2.0)
CODEGEN_SRCS := \
	codegen.c \
	codegen_plugin_common.c \
	codegen_plugin_header.c \
	codegen_plugin_properties.c \
	codegen_export.c \
	codegen_export_backend.c \
	codegen_export_target_c.c

# Top-level targets
.PHONY: all dtd docs docs-index $(MODULES)

all: $(MODULES)

$(CODEGEN): $(CODEGEN_SRCS)
	mkdir -p ../build/bin
	$(CC) $(CFLAGS) $(CODEGEN_EXPORT_CFLAGS) -o $@ $(CODEGEN_SRCS) $(CODEGEN_EXPORT_LIBS)

# Per-module rule — each module builds its 3 files
define MODULE_RULE
$(1): $(CODEGEN)
	@echo "Generating $(1)..."
	$(CODEGEN) -plugin=header     ../$(1).xml > ../$(1).h
	$(CODEGEN) -plugin=properties ../$(1).xml > ../$(1)_properties.h
	$(CODEGEN) -plugin=export     ../$(1).xml > ../$(1)_export.c
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
