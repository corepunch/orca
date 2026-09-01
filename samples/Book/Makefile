.DEFAULT_GOAL := render

SCENER ?= $(shell command -v scener 2>/dev/null || printf '%s/.local/bin/scener' "$$HOME")
LUA ?= lua
ROOMS := $(basename $(notdir $(wildcard Rooms/*.blks)))
ROOM ?= $(ROOMS)
FORMAT ?= jpg
WIDTH ?= 1536
HEIGHT ?= 1024
RENDER_TARGETS := $(addprefix render-,$(ROOM))
SANITY_TARGETS := $(addprefix sanity-,$(ROOM))

LAYOUT_SCALE ?= 2

.PHONY: run render layout sanity measure check-scener $(RENDER_TARGETS) $(LAYOUT_TARGETS) $(SANITY_TARGETS)

LAYOUT_TARGETS := $(addprefix layout-,$(ROOM))

run: render
	$(MAKE) -C ../.. unite
	cd ../.. && build/bin/orca samples/Book

render: $(RENDER_TARGETS)

layout: $(LAYOUT_TARGETS)

sanity: $(SANITY_TARGETS)

measure:
	@test -n "$(OBJECT)" || { echo 'OBJECT is required, e.g. make measure ROOM=workshop OBJECT=SWEEP-BROOM'; exit 2; }
	$(LUA) Tools/measure_scene.lua "Rooms/$(ROOM).blks" "$(OBJECT)" "$(TARGET)"

check-scener:
	@command -v "$(SCENER)" >/dev/null || { \
		echo 'scener is required; install Orion with `make install PREFIX="$$HOME/.local"` and add $$HOME/.local/bin to PATH'; \
		exit 1; \
	}

$(RENDER_TARGETS): render-%: Rooms/%.blks | check-scener
	$(LUA) Tools/check_scene_sanity.lua "$<"
	@mkdir -p "Rooms/render/$*"
	@find "Rooms/render/$*" -maxdepth 1 -type f \( -name '*.jpg' -o -name '*.png' \) ! -name 'layout.*' -delete
	cd Rooms && "$(SCENER)" --render "$*.blks" --size "$(WIDTH)x$(HEIGHT)" \
		--format "$(FORMAT)" --output-dir "render/$*"

$(LAYOUT_TARGETS): layout-%: Rooms/%.blks | check-scener
	$(LUA) Tools/check_scene_sanity.lua "$<"
	cd Rooms && "$(SCENER)" --layout "$*.blks" --scale "$(LAYOUT_SCALE)" \
		--format "$(FORMAT)" --output-dir "render/$*"

$(SANITY_TARGETS): sanity-%: Rooms/%.blks
	$(LUA) Tools/check_scene_sanity.lua "$<"
