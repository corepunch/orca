# Milestone: Wondertown Workshop — Playable Chapter Handover

Status as of 2026-08-27. This document is the handover for the next agent. It
records exactly what the workshop prototype does now, how it is wired, how to run
and test it, and what is deliberately left open.

## Goal of this milestone

Turn the first Wondertown location into a fully playable, Disney-storybook-style
chapter driven by direct Scener renders, while keeping ZIL
authoritative for world state. Scope agreed with the author:

- **Whole workshop building** playable: `WORKSHOP-FLOOR`, `WORKBENCH-TOP`,
  `TOOL-BENCH`, `COUNTERTOP` (plus `STORAGE-LOFT` reachable).
- **FOCUS interaction model** implemented host-side (see
  `libs/zilscript/books/wondertown/FOCUS_INTERACTION_MODEL.md`).
- **Direct PNG renders** from `make render`, one per camera.
- ZIL changes allowed in Infocom style if needed (none were required).

## Mental model

```
            ZIL world (authoritative: HERE, containment, timers, puzzles)
                                   |  parser commands in / result prose out
                         Session brain  (Scripts/WorkshopSession.lua)
                            room  |  focus  |  beat   state
                                   |  view()
                         Host page (Scripts/Start.lua + Screens/Start.xml)
                       establishing hotspots | focus choices | held beat
```

- **World location** stays the normal ZIL `HERE`. It governs exits, objects,
  NPCs, timers.
- **Focus** is presentation only: it narrows the page to one subject's 3–5 local
  choices plus an in-world "step back". Entering a focus never changes `HERE`.
- **Beat**: a command with dedicated action art, or one that changes room, is
  held full-frame with its result prose and a single **Continue**. Quieter
  results just refresh the focused page.

The selected command is the page turn: tap → (focus or take/examine) → beat →
Continue → refreshed establishing page (possibly a new room).

## Files owned by this milestone

Runtime (host):
- `Scripts/WorkshopSession.lua` — the brain. Pure Lua over the ZIL runtime, no
  graphics. Public API below.
- `Scripts/WondertownScenes.lua` — presentation + focus manifest: rooms→camera,
  authored subjects→local choices/exit/action-art, `image(camera)` helper.
- `Scripts/Start.lua` — ORCA UIKit controller. Renders `Session:view()` into
  projected hotspots, text choices, exits, focus choices, and beats.
- `Screens/Start.xml` — page layout (adds a `Choices` StackView).
- `Styles/Start.css` — adds `.choice-option` / `.exit-option`.

Camera projection / export:
- `Scripts/WorkshopInteractions.lua` — camera **export spec** only
  (`scene_path = "Book/Rooms/workshop.blks"`, source resolution, default camera).
- `Tools/export_workshop_camera.lua` — exports **all** story cameras + named
  anchors. Skips the overhead `LayoutPlan` (a vertical look has no yaw
  convention).
- `Scenes/WorkshopCamera.xml`, `Scripts/WorkshopCamera.lua` — generated. Do not
  hand-edit; regenerate with `lua Tools/export_workshop_camera.lua`.
- `Scripts/SceneProjection.lua`, `Scripts/OrcaCameraProjection.lua` — unchanged
  projection math / native-camera adapter.

World (unchanged in this milestone):
- `libs/zilscript/books/wondertown/dungeon.zil` and `actions.zil` — rooms and
  objects already fully authored. `Scripts/WondertownPrototype.zil` is the entry
  point (loads zork1 verbs + wondertown, no `companion.zil`).

Scene blockout:
- `Rooms/workshop.blks` — **not edited** this milestone. Named anchors matching
  ZIL objects: `KEY-HOOK`, `OIL-CAN`,
  `CLOCK-FACE`, `PET-DOOR`. Decoration anchors reused via manifest `node=` alias:
  `main_bench`→`WORKBENCH`, `counter_chair`→`BERTRAND`, `tool_bench`.

## Session API (Scripts/WorkshopSession.lua)

- `Session.new()` → boots ZIL, indexes objects, builds room-id map.
- `:room()` → canonical name of ZIL `HERE` (e.g. `"WORKSHOP-FLOOR"`).
- `:view()` → the single source of truth for rendering. Returns one of:
  - `{kind="room", camera, image, text, subjects={{name,node,kind,command,art,label}}, exits={{label,command}}}`
  - `{kind="focus", subject, camera, image, text, choices={{label,command}}, exit}`
  - `{kind="beat", camera, image, text, continue=true}`
- `:tap(name)` → portable⇒TAKE beat, authored⇒enter FOCUS, else EXAMINE beat.
- `:enter_focus(name)` / `:leave_focus()` (leaving spends no turn).
- `:choose(index)` → run a focus choice. Room change or `art` ⇒ beat; else quiet
  refresh of focus prose/choices.
- `:exit(command)` → room movement. Real move ⇒ next establishing page; refused
  move ⇒ held beat with the reason.
- `:continue()` → dismiss beat; a room-change beat clears focus and lands on the
  new establishing page.

Interaction rule: portable objects (`TAKEBIT`) default to TAKE; a subject with an
entry in `WondertownScenes.focuses` opens FOCUS; anything else EXAMINEs. Subjects
match ZIL objects by name; `node=` aliases a decoration anchor only where the
blockout name differs.

## Cameras → rooms/beats

| Camera | Use |
|---|---|
| `WorkshopEstablishing` | WORKSHOP-FLOOR establishing |
| `EmptyHookReveal` | KEY-HOOK focus |
| `ClimbWorkbenchAction` | WORKBENCH focus + climb beat |
| `WorkbenchTopEstablishing` | WORKBENCH-TOP establishing + close-book beat |
| `RepairBookCloseup` | REPAIR-BOOK focus + open beat |
| `OilCanCloseup` | oil-can take beat |
| `ToolBenchEstablishing` | TOOL-BENCH establishing |
| `WindBertrandAction` | BERTRAND focus + wind beat |
| `MakeshiftClimbAction` | MAKESHIFT-STEPS focus + climb beat |
| `CountertopEstablishing` | COUNTERTOP establishing + case/doll focus |
| `LayoutPlan` | overhead reference; NOT exported/used |

## Playable path (verified end-to-end headlessly)

Floor: focus the empty hook → take the oil can (beat) → focus the workbench →
climb (beat) → **WORKBENCH-TOP**. Open the repair book (reveal beat); trying to
climb down while open is refused; close it → climb **down** to the floor →
**east** to the **TOOL-BENCH**. Focus Bertrand → take the winding key → wind him
(beat, way up clears) → focus the makeshift steps → climb (beat) →
**COUNTERTOP** → open the display case.

## How to run and test

See `RENDERING.md` for the complete Scener rendering workflow, direct
single-camera commands, validation, and troubleshooting.

Headless logic (works anywhere with `lua` 5.4):

```sh
# from samples/Book
lua Tests/test_scene_projection.lua
lua Tests/test_camera_export.lua
# whole-chapter Session walk (run from the zilscript lib so requires resolve)
cd libs/zilscript && lua ../../Tests/test_workshop_prototype.lua
```

All three currently PASS.

Render the art (needs a graphics session and working OpenGL context):

```sh
# from samples/Book
export PATH="$HOME/.local/bin:$PATH"
make check-scener
make render ROOM=workshop            # -> Rooms/render/workshop/<Camera>.png
```

Run the app (builds ORCA + plugins, needs a graphics session):

```sh
# from samples/Book
make run
```

GUI-level tests need the ORCA + GL build (cannot run in a headless session):

```sh
# from the orca repository root
build/bin/orca -test=samples/Book/Tests/test_workshop_ui.lua
build/bin/orca -test=samples/Book/Tests/test_camera_export_native.lua
```

Regenerate the camera export after any change to `Rooms/workshop.blks`:

```sh
lua Tools/export_workshop_camera.lua   # rewrites Scenes/WorkshopCamera.xml + Scripts/WorkshopCamera.lua
```

## Environment note

Scener rendering and `make run` require a graphical session. Missing renders are
reported as load errors; previous artwork is never used as a fallback.

## Known gaps / next steps

1. **Regenerate and review the scenes after scene changes.** The manifest expects
  `Rooms/render/workshop/<Camera>.png`. Run
   `make render ROOM=workshop`, then `make run`, and verify hotspot registration
   for each camera. Cameras need a visibility review: projecting a point cannot
   tell whether furniture hides it.
2. **Verify GUI tests** on an ORCA build (`test_workshop_ui.lua`,
   `test_camera_export_native.lua`).
3. **Oil-can → loft-ladder → storage-loft branch is inert.** `Rooms/workshop.blks`
   has no loft-ladder geometry, so `LOFT-LADDER`/`LADDER-MECH` are not subjects or
   a focus. To wire it: add ladder geometry + a named anchor, then a focus with
   choices `examine mechanism` / `lubricate mechanism` / `up` (use the verb
   `LUBRICATE`, not `OIL`, which is also an oil-can adjective). ZIL already
   supports the puzzle (`V-OIL`, `LADDER-OILED`, `UP TO STORAGE-LOFT IF
   LADDER-OILED`).
4. **COUNTERTOP interior is not modelled.** `MARZIPAN`, `DISPLAY-CASE`,
   `SHOP-WINDOW` have no scene geometry, so they render as text choices (menu
   fallback) rather than projected hotspots until the countertop is blocked out.
5. **Optional: rename decoration anchors to ZIL names** (`main_bench`→`WORKBENCH`,
   `counter_chair`→`BERTRAND`) in `Rooms/workshop.blks` and drop the manifest
   `node=` aliases, to fully honor same-name matching. Re-export afterward.
6. **Add remaining rooms/books** by following the same manifest pattern
   (`rooms`, `focuses`, `exits`); no host code changes are required for content
   that reuses TAKE/EXAMINE/FOCUS and cardinal exits.

## Design invariants to preserve

- ZIL owns world logic and result prose. Every host choice is an ordinary parser
  command; do not encode puzzle logic in the manifest.
- Focus is presentation only. Never assign a focused object to `HERE`.
- Reading room prose or leaving a focus must not issue `LOOK` or spend a turn.
- Scener renders are blocking art, not finished illustration; keep the exported
  camera and object silhouettes for registration when art is commissioned.
