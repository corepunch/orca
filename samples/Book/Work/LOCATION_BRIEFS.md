# Location Sources and Connected Scene Briefs

Use this guide before generating a room. The aim is a continuous place with
several authored fixed-camera views, not unrelated illustrations for each parser
location. Composition rules live in [SCENE_COMPOSITION.md](SCENE_COMPOSITION.md).

## Sources and ownership

| Need | Source | How to use it |
|---|---|---|
| Implemented locations, exits, objects and initial state | [Wondertown dungeon.zil](../libs/zilscript/books/wondertown/dungeon.zil) | Read the relevant ROOM and OBJECT declarations and referenced conditions/actions. These decide what exists and how travel works. |
| Book interaction and camera selection | [PROCESS.md](PROCESS.md), `../Scripts/WondertownScenes.lua`, `../Scripts/WorkshopInteractions.lua` | Verify the host's supported slice, focus choices, image names and export settings. |
| Narrative geography overview | [Wondertown map](../libs/zilscript/books/wondertown/work/MAP.md) | Design context only; it contains directions and omissions that differ from implemented ZIL. Resolve against source before building. |
| Appearance and cast | [ARTSTYLE.md](ARTSTYLE.md), [CHARACTER_DESIGN_BIBLE.md](CHARACTER_DESIGN_BIBLE.md), `references/` | Book's approved visual identity; retain story-defined scale and object function. |
| Current geometry | `../Rooms/<scene>.blks`, `../Rooms/prefabs/` | Inspect current transforms and cameras before applying historical layout notes. |
| Fixed-camera reference | `~/Developer/Temp/AITD/output/` | Record exact export and camera IDs; see the composition guide's coordinate caveat. |
| Experiments and old proposals | [WORKSHOP_LAYOUT_NOTES.md](WORKSHOP_LAYOUT_NOTES.md), [MANSION_STUDY_COMPOSITION.md](MANSION_STUDY_COMPOSITION.md) | Supporting notes, not authority for new story content or current implementation status. |

Do not generate a location from its name alone. Capture room prose, visible
objects, approaches, exits and relevant changed states together. Follow only the
routines needed to resolve the selected scene. Unresolved geography goes in the
brief as an open design decision, not as an invented passage.

## Wondertown's first connected set

The current story supports the following physical grouping. This is a planning
map, not a claim that every view or scene is already implemented in Book.

| ZIL location | Physical interpretation | Continuity to preserve |
|---|---|---|
| WORKSHOP-FLOOR | Main workshop floor | Empty hook/string, oil can beneath bench, clock, folding ladder/mechanism, north pet door inside main door. |
| WORKBENCH-TOP | Top of the same workbench | Same bench below and above; repair book, tools, shavings and unfinished toys retain placement. |
| TOOL-BENCH | East work area within the workshop | No new architectural doorway follows merely from an EAST parser exit. Keep tool rack, Bertrand and climb furniture in the same shell. |
| COUNTERTOP | Elevated display at tool-bench area | Low crate → chair → three repair books → countertop; frosted shop window, display case and doll agree with views from below. |
| STORAGE-LOFT | Raised storage above workshop | Ladder reaches an opening/landing; folded and deployed states match the mechanism and access condition. |
| SNOWY-ALLEY | Exterior behind workshop | Both sides of pet/main door align; moonlight and threshold elevation agree. |
| TOLLIVER-STUDY | Distinct space reached from workshop | Workshop IN requires STUDY-ACCESS; source describes stairs on return. Determine footprint/elevation before assigning an opening. |
| WORKSHOP-HEART | Hidden chamber behind the clock | Implemented travel goes down from study. Resolve the chamber's physical relationship before constructing passages. |

Use source-backed dressing first: chisels, files and hammers on the tool rack;
a varnish work area; tools and shavings on the bench; the green leather repair
book; half-finished sailboat, train and one-wheeled duck; display furniture by
the window. These give each zone a purpose and a distinct silhouette palette.
Decoration must not introduce a new puzzle object or imply an unavailable exit.

## Brief template

Create a brief for each connected set before its production pass. Keep it short
and update it with the scene. Coordinates are design decisions until checked in
the scene and rendered; do not label guessed measurements as source facts.

```text
Connected set / scene file:
Source: story file + ROOM/OBJECT IDs + relevant routine/state references
Physical zones and shared origin (X east, Y north, Z up, centimetres):
Occupant scale and human furniture reference:
Connections: from → to; direction; doorway/open zone/climb; access condition
Thresholds: world position/orientation, width/height, elevations, swing/landing
Required objects: ID, support/parent, stable position, visible state
Dressing clusters: activity, supporting props, density, intentional quiet areas
Room identity: architecture, materials, palette, distinctive recurring landmark
Lights: source/fixture, position/direction, color, shadow purpose, shared time
Shots: camera ID, role, target, entry/exit direction, required readable subjects
UI: render dimensions, runtime crop, prose/choices/Continue footprints
State variants: what changes, what stays fixed, image/camera/export mapping
References: exact file/camera/image, borrowed quality, properties to preserve
Open questions: unsupported source detail or renderer capability
Acceptance: plan + travel contact sheet + state pair + real UI crop
Revision: scene/prefab revision, renderer revision, commands, output paths
```

A useful generation request names these constraints, for example:

> Dress the existing workshop's tool-bench zone as an active repair station,
> using the declared rack tools, varnish, Bertrand and countertop climb. Keep
> its shared floor, furniture positions and route from workshop-floor. Group
> tools by activity, give the bench a readable working surface, and keep the
> crate/chair/books usable. Compare an oblique approach and a low climb view.
> Carry the same practical lights and window direction across both cameras;
> reserve the actual UI footprint. Do not add a doorway for the parser's EAST.

Review generation in stages: geography → silhouettes/cameras → dressing →
lighting/materials → connected sequence and UI. Carry accepted decisions forward
rather than regenerating an independently redesigned room for every shot.
