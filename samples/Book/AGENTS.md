# Book Agent Guide

## Start with the task

This directory owns Book's visual scenes, rendering pipeline and presentation.
The current scene goal is appealing, connected 3D locations viewed through
fixed cameras, with *Alone in the Dark* / *Resident Evil* staging references.
Wondertown keeps its approved storybook identity. Read only the guides needed
for the change, then inspect the targeted source/scene and one relevant example.

| Need | Read |
|---|---|
| What to generate; source material; connected location brief | [Work/LOCATION_BRIEFS.md](Work/LOCATION_BRIEFS.md) |
| Room density, geography, camera placement, composition, lighting/shadows and acceptance | [Work/SCENE_COMPOSITION.md](Work/SCENE_COMPOSITION.md) |
| Scener build, CLI compatibility, rendering, export, validation and redeployment | [Work/RENDERING.md](Work/RENDERING.md) |
| Book interaction, focus/action beats and image workflow | [Work/PROCESS.md](Work/PROCESS.md) |
| Existing native Orca camera/anchor scene and interaction UI | [Scenes/WorkshopCamera.xml](Scenes/WorkshopCamera.xml), [Screens/Start.xml](Screens/Start.xml), [Scripts/Start.lua](Scripts/Start.lua) |
| Wondertown finish, materials, palette and illustration prompts | [Work/ARTSTYLE.md](Work/ARTSTYLE.md) |
| Locked cast identities and scale | [Work/CHARACTER_DESIGN_BIBLE.md](Work/CHARACTER_DESIGN_BIBLE.md), `Work/references/` |
| Fresh workshop assets, camera coverage and rebuild commands | [Work/workshop-new/DESIGN.md](Work/workshop-new/DESIGN.md) |
| Workshop source corrections and next visual pass | [Work/WORKSHOP_LAYOUT_NOTES.md](Work/WORKSHOP_LAYOUT_NOTES.md) |
| Historical mansion study experiment | [Work/MANSION_STUDY_COMPOSITION.md](Work/MANSION_STUDY_COMPOSITION.md) |
| Prototype milestone context | [MILESTONE.md](MILESTONE.md) |
| Story or parser changes under the embedded library | [libs/zilscript/AGENTS.md](libs/zilscript/AGENTS.md), then its relevant source/docs |

`Work/` holds canonical visual guides. The root `PROCESS.md`,
`SCENE_COMPOSITION.md` and `CHARACTER_DESIGN_BIBLE.md` are compatibility links;
do not restore competing copies. Historical examples and milestones do not
supersede current story source, scene files or the host manifest.

## Scene ownership and invariants

- Standard workflow: author `.blks`/`.blk` geometry in Scener, render `.jpg`
  backgrounds, export matching cameras and anchors into native Orca XML, then
  integrate and review them in the existing `samples/Book` runtime. Use
  `Scenes/WorkshopCamera.xml`, `Screens/Start.xml`, `Scripts/Start.lua` and
  `Scripts/WorkshopSession.lua` as the workshop implementation to extend.
  New artwork must use this same interaction pipeline; do not build a separate
  viewer or navigation prototype. Keep images, camera axes, dimensions and
  anchors synchronized, and verify hotspots, focus, Back and Continue in Book.
- Do not produce SVG files for scene output, camera reviews or navigation
  overlays. Deliver `.blks` scenes, `.blk` prefabs and `.jpg` renders; review
  images and contact sheets must be raster images. Reuse the existing native
  Orca XML scene and Book UI for projected hotspots, focus, Back and Continue.
  Native Orca XML camera/anchor exports remain part of runtime integration.
- `libs/zilscript/books/wondertown/dungeon.zil` owns implemented rooms, objects,
  exits and state rules. A ZIL room may be a zone or elevated surface inside a
  shared physical room. Do not invent an architectural door for every exit.
- `Rooms/*.blks` and `Rooms/prefabs/` own shared 3D geometry. Use centimetres,
  X east, Y north, Z up, degrees and unitless scale. Keep connected zones in a
  common coordinate system, or record the transform between their scenes.
- Block the large masses and routes first, then add designed furniture and
  purposeful dressing. The early 10 cm silhouette shortcut is not a ban on
  small finished geometry or Pip-scale objects. Inspect density at delivery size.
- Name interactive groups with exact ZIL IDs. Camera names used for runtime
  screenshots follow the existing lower-case, hyphenated room/object IDs;
  verify `Scripts/WondertownScenes.lua` before changing an image name.
- The establishing view covers every current-location interaction. Show two
  populated wall planes plus a back affordance, or three walls from a camera
  against the fourth. Focus shots supplement the overview; they do not conceal
  ordinary room affordances. Navigation arrows belong in UI, not JPEG art.
- Keep permanent geometry and world lighting consistent across shots. Record
  changed story states explicitly. Check both sides of room connections and
  review forward/reverse camera sequences with real UI overlays.
- `Scripts/WorkshopInteractions.lua` specifies camera export input/dimensions;
  `Scripts/WondertownScenes.lua` maps presentation/focus choices. Regenerate
  camera metadata and matching state images together as described in RENDERING.
- Local AITD exports are at `~/Developer/Temp/AITD/output/` (other numbered
  variants also exist). Use identified files/cameras as references; verify axis,
  scale and FOV conversion. Proxy exports are not finished lighting/density art.

## Scener is a separate working checkout

Source lives at `~/Developer/mapview/ui/apps/scener`, with build root
`~/Developer/mapview/ui`. Read that checkout's instructions before edits.
Changes to Book scenes stay here; Scener CLI, renderer and editor fixes belong
there. Build with `make -C "$HOME/Developer/mapview/ui" build/bin/scener`.

The Book-compatible CLI and macOS deployment helper are documented in
[Work/RENDERING.md](Work/RENDERING.md) and the Scener checkout's `CLI.md`.
Use `python3 apps/scener/deploy.py --prefix "$HOME/.local"` from the UI root
following its build and tests. The launcher includes runtime-library handling;
do not copy only the binary. New Z-up Book scenes must declare `up="z"` while
preserving native primitive local axes. Validate the deployed tool from Book
at the final raster size and inspect shadows across several cameras.

For scene changes, run focused sanity/measurement checks, render layout and
changed cameras with a compatible Scener, and inspect the results. For camera
or anchor changes, also regenerate metadata and run the documented projection
checks. For documentation-only changes, check links, command accuracy and
contradictory guidance; no renderer rebuild is needed.
