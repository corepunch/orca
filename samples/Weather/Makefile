APP_NAME = orca
BUILD_DIR = ../../build/bin
DATA_DIR = .
LIB_PATH = $(BUILD_DIR)

.PHONY: run

run:
	@echo "Running WeatherTabs sample..."
	DYLD_LIBRARY_PATH=$(LIB_PATH) $(BUILD_DIR)/$(APP_NAME) $(DATA_DIR)
