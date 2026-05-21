ROOT := ../..

.PHONY: all clean run-sample run-sample-properties run-sample-docs run-sample-dtd

all:
	$(MAKE) -C $(ROOT) codegen-host

run-sample:
	$(MAKE) -C $(ROOT) codegen-sample

run-sample-properties:
	$(MAKE) -C $(ROOT) codegen-host
	@mkdir -p build
	$(ROOT)/build/bin/cgen $(ROOT)/source/core/core.cgen $(ROOT)/build/plugins/codegen/libproperties.so > build/core_stub_properties.h

run-sample-docs:
	$(MAKE) -C $(ROOT) codegen-host
	@mkdir -p build
	$(ROOT)/build/bin/cgen $(ROOT)/source/core/core.cgen $(ROOT)/build/plugins/codegen/libdocs.so > build/core_stub.md

run-sample-dtd:
	$(MAKE) -C $(ROOT) codegen-host
	@mkdir -p build
	$(ROOT)/build/bin/cgen $(ROOT)/plugins/UIKit/UIKit.cgen $(ROOT)/build/plugins/codegen/libdtd.so > build/ui_stub.dtd

clean:
	$(MAKE) -C $(ROOT) codegen-clean
