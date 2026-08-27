.DEFAULT_GOAL := render

SCENER ?= $(shell command -v scener 2>/dev/null || printf '%s/.local/bin/scener' "$$HOME")
ROOMS := $(basename $(notdir $(wildcard Rooms/*.blks)))
ROOM ?= $(ROOMS)
FORMAT ?= jpg
WIDTH ?= 1536
HEIGHT ?= 1024
RENDER_TARGETS := $(addprefix render-,$(ROOM))

.PHONY: run render check-scener $(RENDER_TARGETS)

run: render
	$(MAKE) -C ../.. unite
	cd ../.. && build/bin/orca samples/Book

render: $(RENDER_TARGETS)

check-scener:
	@command -v "$(SCENER)" >/dev/null || { \
		echo 'scener is required; install Orion with `make install PREFIX="$$HOME/.local"` and add $$HOME/.local/bin to PATH'; \
		exit 1; \
	}

$(RENDER_TARGETS): render-%: Rooms/%.blks | check-scener
	cd Rooms && "$(SCENER)" --render "$*.blks" --size "$(WIDTH)x$(HEIGHT)" \
		--format "$(FORMAT)" --output-dir "render/$*"
