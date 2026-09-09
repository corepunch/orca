# New workshop: the inhabited room

Fresh scene and prefab family, created from the Wondertown story rather than the
old workshop geometry. Coordinates: X east, Y north, Z up, centimetres.
`Rooms/workshop-new.blks` explicitly declares `up="z"` for Scener.

The workshop is 860 × 1030 cm (Y = −270…760), with a 400 cm ceiling. A high camera inside the south
wall sees three walls: a carved working bench and missing-key station to the
left, the rear pet door and storage loft ahead, and the tool bench/display to
the right. A small stocked repair trolley presents its top in the foreground.
The central floor remains walkable. The three primary visual directions are
left to the workbench, ahead to the pet door, and right to the tool bench;
upward travel to the loft is a fourth, conditional route.

The establishing shot is the interaction overview. Every workshop-floor subject
must be visible there, including the oil can underneath the bench's open front.
Detail cameras supplement it. The back affordance belongs to the presentation
layer, in a reserved lower-left screen corner, and returns each focus shot to
its physical parent area; it is not a painted sign or invented world exit.

Source: `libs/zilscript/books/wondertown/dungeon.zil`, workshop floor, workbench
top, tool bench, countertop, storage loft and their objects. The initial story
state has an empty hook with string, an available oil can, a closed repair book,
a locked ladder mechanism and dormant toys. Scene decoration is noninteractive.

Architecture and furniture are new design decisions: warm plaster above teal
wainscot, thick oak joinery, Roman-arched openings, shallow recessed shelving,
a loft supported on posts, a copper/brass practical over each working zone.
The frosted shop window supplies cool contrast; practical lamps light the
work surfaces and cast grounded shadows. Floorboard variation and grouped
shavings create use and age without scattering high-contrast debris everywhere.

Density passes: shell and trim; distinctive benches/clock/loft; stocked shelves
and open furniture bays; bench toys/tools/books; small fittings and shavings.
Keep every cluster physically supported and preserve identifying silhouettes.
Use 4:3 delivery at 1920 × 1440 to fit the Book page without horizontal cropping.

Coverage includes all five interior location cameras, every authored focus
subject in the current Book manifest, and a threshold/reverse view. Focus images
are object coverage; they do not claim to depict animated actions or character
performances. Future book-open, oil-removed and ladder-deployed variants must
follow ZIL state and keep all permanent furniture fixed.

## Reproduction and review

See [VALIDATION.md](VALIDATION.md) for delivered image checks and their limits.

```sh
# From samples/Book; use the deployed Scener described in ../RENDERING.md.
python3 Tools/build_workshop_new.py
make sanity ROOM=workshop-new
make render ROOM=workshop-new WIDTH=1920 HEIGHT=1440
make layout ROOM=workshop-new
```

The new family is `Rooms/prefabs/workshop-new/`, independent of the former
workshop assets. `build_workshop_new.py` regenerates architecture, furniture,
fixtures and the scene, leaving the hand-authored `props/` family intact.

Review `Rooms/render/workshop-new/workshop-floor.jpg` and `contact.jpg` for
camera coverage. Do not generate SVG reviews or overlays. Navigation and Back
controls belong in Book's UI and must respect ZIL access conditions. The `.jpg`
renders contain no UI.

This workshop is now the active Book scene. Run `make run` from `samples/Book`
to render, export native metadata, build and launch it. No action-animation
frames or changed-state variants are claimed.

Orca already provides the interaction host: `Scenes/WorkshopCamera.xml` holds
native cameras and anchors, and `Screens/Start.xml` with `Scripts/Start.lua`
displays projected hotspots, focus choices, return controls and Continue.
Reuse this implementation when integrating the new workshop.

The runtime reads `Rooms/render/workshop-new/`.
`Scripts/WorkshopInteractions.lua` selects `Rooms/workshop-new.blks` at
1920 × 1440. The native exporter converts Z-up camera positions, look targets
and anchors together into Orca Y-up coordinates `(x, z, -y)`. The source
projection uses Z up, preserving agreement with the rendered images.

The existing Lua sanity check currently sees only its supported box groups;
it does not validate the internal geometry of these composite prefabs. XML,
reference resolution, manual support/contact checks and every-camera visual
review therefore remain necessary alongside its successful exit status.
