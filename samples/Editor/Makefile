APP_NAME = orca
BUILD_DIR = ../../build/bin
DATA_DIR = .
LIB_PATH = $(BUILD_DIR)

# Targets
.PHONY: run clean

help:
	@echo "Available targets:"
	@echo "  run               - Run the game in GUI mode"
	@echo "  clean             - Clean up build artifacts"

run:
	@echo "Setting DYLD_LIBRARY_PATH to $(LIB_PATH) and running $(APP_NAME)..."
	DYLD_LIBRARY_PATH=$(LIB_PATH) $(BUILD_DIR)/$(APP_NAME) $(DATA_DIR)

cluster:
	@echo "Setting DYLD_LIBRARY_PATH to $(LIB_PATH) and running $(APP_NAME)..."
	DYLD_LIBRARY_PATH=$(LIB_PATH) $(BUILD_DIR)/$(APP_NAME) -lib=$(BUILD_DIR) -data=$(CLUSTER_DIR)

copy-resources:
	@echo "Copying contents of current directory to $(RESOURCES_DIR)..."
	@mkdir -p $(RESOURCES_DIR)  # Ensure the target directory exists
	@cp -r ./* $(RESOURCES_DIR)

clean:
	@echo "Cleaning up..."
	# Add cleaning commands here if needed

