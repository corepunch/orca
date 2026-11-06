# ORCA Editor. PROJECT selects the project to edit (default: ../Example).
BUILD_DIR = ../../build
PROJECT ?= ../Example

.PHONY: run help

help:
	@echo "Available targets:"
	@echo "  run [PROJECT=path]  - Open the editor on a project (default ../Example)"

run:
	ORCA_EDITOR_PROJECT=$(abspath $(PROJECT)) \
	LD_LIBRARY_PATH=$(BUILD_DIR)/lib DYLD_LIBRARY_PATH=$(BUILD_DIR)/lib \
	$(BUILD_DIR)/bin/orca .
