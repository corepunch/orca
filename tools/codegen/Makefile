# Makefile for tools/codegen

CODEGEN := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/codegen)

.PHONY: install test

install:
	chmod +x $(CODEGEN)

test:
	@echo "Testing geometry header..."
	@$(CODEGEN) ../../source/geometry/geometry.xml - header | \
		diff - ../../source/geometry/geometry.h && echo "  geometry.h OK"
	@echo "Testing geometry properties..."
	@$(CODEGEN) ../../source/geometry/geometry.xml - properties | \
		diff - ../../source/geometry/geometry_properties.h && echo "  geometry_properties.h OK"
	@echo "Testing geometry export..."
	@$(CODEGEN) ../../source/geometry/geometry.xml - export | \
		diff - ../../source/geometry/geometry_export.c && echo "  geometry_export.c OK"
	@echo "Testing core header..."
	@$(CODEGEN) ../../source/core/core.xml - header | \
		diff - ../../source/core/core.h && echo "  core.h OK"
	@echo "Testing core properties..."
	@$(CODEGEN) ../../source/core/core.xml - properties | \
		diff - ../../source/core/core_properties.h && echo "  core_properties.h OK"
	@echo "Testing core export..."
	@$(CODEGEN) ../../source/core/core.xml - export | \
		diff - ../../source/core/core_export.c && echo "  core_export.c OK"
