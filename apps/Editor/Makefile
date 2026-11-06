# ORCA Editor. PROJECT preselects a project in the welcome screen.
BUILD_DIR = ../../build

.PHONY: run help

help:
	@echo "Available targets:"
	@echo "  run [PROJECT=path]  - Show the welcome screen, optionally preselecting a project"

run:
	$(if $(PROJECT),ORCA_EDITOR_PROJECT=$(abspath $(PROJECT))) \
	LD_LIBRARY_PATH=$(BUILD_DIR)/lib DYLD_LIBRARY_PATH=$(BUILD_DIR)/lib \
	$(BUILD_DIR)/bin/orca .
