# Workshop Floor — Layout Notes

Analysis of `Rooms/workshop.blks` against the ZIL room descriptions and
`SCENE_COMPOSITION.md`. Records what is wrong, what is missing, and what
to add for flavor before the next render pass.

## What the room says it contains

From `dungeon.zil` WORKSHOP-FLOOR:

> "Grandfather Tolliver's workshop. Sawdust covers the floor like a golden
> blanket. The brass key hook on the wall is empty — only a frayed string
> hangs from it. A pet door glows with moonlight to the north."

Named objects in this room: `KEY-HOOK`, `WORKBENCH`, `OIL-CAN` (beneath
the bench), `CLOCK-FACE` (cuckoo clock on the wall), `PET-DOOR` (north
wall base), `LOFT-LADDER` (folding, against the wall), `BROOM` (leaning
against the workbench).

Exits: NORTH (to SNOWY-ALLEY via pet door), EAST (to TOOL-BENCH),
UP (to STORAGE-LOFT when `LADDER-OILED`).

## Issues in the current blockout

### 1 — Crate stack is floating

`<box pos="300 390 70" size="100 100 140"/>` sits 250 cm from the east
wall inner face with no wall relationship. A storage crate should be
pushed against the nearest wall. Correct X from 300 to ~490 (gap ≈ 80 cm
to inner east wall at X=576).

### 2 — Tool counter has an unnecessary gap

`<box pos="430 720 46" size="200 90 92"/>` leaves ≈ 46 cm between its
east face and the east wall inner surface. A tool counter is worked from
the front; push it flush. Correct X from 430 to 476 (right edge ≈ X=576).

### 3 — No east-wall exit opening

ZIL exits EAST to TOOL-BENCH, but the east wall has no arch or opening.
Add a `bool-negative-arch` in the east wall at the south end (around
X=600, Y=200–400) so the exit reads visually as a passage and the camera
can see depth into the next room.

### 4 — Window is a placeholder box, not a window prefab

`<box pos="588 820 280" size="24 200 160"/>` is sitting inside the east
wall as a rough placeholder. The `PROCESS.md` describes the window prefab
as containing a `bool-negative-arch`, four perimeter rails, inset pane,
and mullions, with outer dimensions 2.0 × 1.7 units. Replace the box with
the proper window prefab instance so the opening actually cuts the wall and
`cool moonlight entering horizontally through the back window` (PROCESS.md)
reads as motivated light.

### 5 — West wall is completely bare

The west wall has no geometry. The KEY-HOOK and CLOCK-FACE are described
as being "on the wall" — they should be placed as named scene anchors. The
cuckoo clock in particular needs a silhouette (see SCENE_COMPOSITION on
10 cm minimum) so it reads from the establishing camera.

### 6 — LOFT-LADDER is missing

`LOFT-LADDER` appears in `dungeon.zil` and in the MILESTONE as a story
object whose absence blocks the `OIL-CAN → lubricate mechanism → up`
puzzle. SCENE_COMPOSITION lists it as a "large semantic form" that must be
modelled. Place it folded against the west or north-west wall beneath the
loft platform (`Z ≈ 0–640`, `Y ≈ 800–940`, `X ≈ −580`).

## Proposed additions for flavor

These are below the 10 cm silhouette threshold for small ornament, but the
items below either (a) explain motivated light, (b) represent a named
story anchor, or (c) break up otherwise featureless wall planes.

### Windows

| # | Wall | Proposed position | Purpose |
|---|------|-------------------|---------|
| A | East (existing, fix prefab) | X=576, Y=720–920, Z=160–330 | Primary back window; moonlight pool on floor |
| B | West (new) | X=−576, Y=500–640, Z=200–360 | Secondary cross-light; breaks bare west wall |

Window B is optional flavor — it is not mentioned in the ZIL or PROCESS.md.
Omit it if motivated lighting from two sources is enough.

### Hanging lamps

Two warm practicals, as described in PROCESS.md ("hanging-lamp prefab
contains its cord, shadow-casting shade, visible bulb, and point light"):

| # | Position | Lights |
|---|----------|--------|
| Lamp 1 (key) | X=−200, Y=250, Z=580 | Above main workbench — key light for establishing |
| Lamp 2 (fill) | X=450, Y=700, Z=580 | Above tool counter — secondary practical |

### Named wall anchors

These must appear as named groups (or prefab instances with a named group
wrapper) so `Tools/export_workshop_camera.lua` can pick them up.

| Object | Suggested wall | Suggested position | Notes |
|--------|---------------|--------------------|-------|
| `CLOCK-FACE` | West wall | X=−576, Y=500, Z=390 | Needs a shaped case silhouette (see PROCESS.md — "give the clock an authored antique cuckoo-clock case") |
| `KEY-HOOK` | West wall | X=−576, Y=250, Z=360 | Brass wall hook; small but story-critical |
| `OIL-CAN` | Beneath workbench | X=−150, Y=90, Z=14 | Under the bench overhang, visible in `OilCanCloseup` |
| `PET-DOOR` | North wall arch base | X=−170, Y=1000, Z=0 | At floor level inside the arch opening |

### Wall shelves

A shelf run along the west wall above waist height (X=−576, Y=200–700,
Z=340–420, two boards) gives tools and toys a plausible home and adds depth
to the left-hand wall plane without blocking the camera path or the clock.
Use the `shelf` prefab.

### Loft-ladder

Folding ladder stored upright against the west wall. When oiled and
deployed it should visually reach the loft platform (`Z ≈ 500–520`):

- Stored (default state): pos=−575 Y=880 Z=300, vertical, width ≈ 40 cm
- Deployed: leaning at angle, top resting on loft edge

Only the stored state needs to be in the blockout; the deployed state is
an action-art camera (`MakeshiftClimbAction` pattern).

## Corrected positions summary

| Object | Current pos (X Y Z) | Corrected pos | Reason |
|--------|---------------------|---------------|--------|
| Crate stack | 300 390 70 | **490 390 70** | Push against east wall |
| Tool counter | 430 720 46 | **476 720 46** | Flush to east wall inner face |
| Window | 588 820 280 (box) | replace with window prefab | Cuts wall, motivated light |
| East-wall exit | missing | bool-negative-arch at ~X=600, Y=250, Z=0–300 | Reads exit to TOOL-BENCH |
| LOFT-LADDER | missing | add near west wall Y=840–940 | Story object, puzzle required |
| CLOCK-FACE | missing | add at X=−576, Y=500, Z=390 | Named anchor |
| KEY-HOOK | missing | add at X=−576, Y=250, Z=360 | Named anchor |
