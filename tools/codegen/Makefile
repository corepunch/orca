CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11 -fPIC
LDFLAGS ?=

ROOT := ../..
BUILD := build
BIN := $(BUILD)/cgen
PLUGDIR := $(BUILD)/plugins

XML_CFLAGS := $(shell pkg-config --cflags libxml-2.0)
XML_LIBS := $(shell pkg-config --libs libxml-2.0)

INCLUDES := -Iinclude -Isrc $(XML_CFLAGS)
CORE_SRC := src/main.c src/model.c src/util.c
CORE_OBJ := $(CORE_SRC:%.c=$(BUILD)/%.o)

PLUGINS := header_stub header properties export
PLUGIN_SO := $(PLUGINS:%=$(PLUGDIR)/%.so)

.PHONY: all clean run-sample run-sample-properties

all: $(BIN) $(PLUGIN_SO)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BIN): $(CORE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@ $(XML_LIBS) -ldl $(LDFLAGS)

$(PLUGDIR)/%.so: plugins/%.c include/cg_api.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -shared $< -o $@

run-sample: all
	$(BIN) --xml $(ROOT)/source/core/core.xml --plugin $(PLUGDIR)/header.so --out $(BUILD)/core_stub.h

run-sample-properties: all
	$(BIN) --xml $(ROOT)/source/core/core.xml --plugin $(PLUGDIR)/properties.so --out $(BUILD)/core_stub_properties.h

clean:
	rm -rf $(BUILD)
