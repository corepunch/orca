.DEFAULT_GOAL := render

SKETCH := libs/simplesketch3d
ROOMS := $(basename $(notdir $(wildcard Rooms/*.blks)))
ROOM ?= $(ROOMS)
FORMAT ?= jpg
WIDTH ?= 1536
HEIGHT ?= 1024
RENDER_TARGETS := $(addprefix render-,$(ROOM))

.PHONY: run render renderer $(RENDER_TARGETS)

run:
	$(MAKE) -C ../.. unite
	cd ../.. && build/bin/orca samples/Book

render: $(RENDER_TARGETS)

renderer:
	$(MAKE) -C $(SKETCH) screenshot

$(RENDER_TARGETS): render-%: Rooms/%.blks | renderer
	cd Rooms && ../$(SKETCH)/build/bin/screenshot "$*.blks" -all \
		-format "$(FORMAT)" -w "$(WIDTH)" -h "$(HEIGHT)" -d 24 -o "render/$*"
