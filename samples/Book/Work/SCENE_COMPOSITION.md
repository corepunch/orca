# Scene and Camera Composition

Canonical guide for Book's connected 3D environments and fixed-camera
backgrounds. Use *Alone in the Dark* and *Resident Evil* as references for
architecturally placed cameras, controlled reveals, occupied rooms and coherent
travel. These are staging references; Wondertown's identity remains in
[ARTSTYLE.md](ARTSTYLE.md) and [CHARACTER_DESIGN_BIBLE.md](CHARACTER_DESIGN_BIBLE.md).
A mansion study is a separate visual exercise, not permission to turn
Wondertown into a horror mansion.

Read [LOCATION_BRIEFS.md](LOCATION_BRIEFS.md) to decide **what exists**;
[RENDERING.md](RENDERING.md) owns commands, renderer capabilities and deployment.

## Build a place before choosing its pictures

A ZIL room can be a tabletop, a work area or a floor within one physical room.
Do not turn every parser exit into a doorway. Map the relevant story locations
onto a shared physical plan before adding architecture. Keep contiguous areas
in one scene where practical; separate files need an explicit common origin or
recorded transform and matching threshold dimensions.

For each connection record both location IDs, direction, passage type, clear
width/height, floor elevations, door hinge/swing or climb endpoints, and any
state that permits travel. The same door has the same position, proportions,
trim and state from both sides. Model enough of the adjoining space to make a
view through an opening convincing; an unexplained black rectangle is not a
connected room. Use a real turn, door or lighting falloff to limit the view.

Keep permanent furniture and dressing fixed between cameras. Changes such as a
removed oil can, opened book or deployed ladder belong to explicit story states.
A close-up must not silently move the subject to a better-lit part of the room.

## Coordinate convention and scale

Book uses X east, Y north/depth, Z up; distances are centimetres, rotations are
degrees, and scale is unitless. Z=0 is the local floor datum. Record the origin
for each connected set; north is shared, not redefined for each camera.
Declare `up="z"` in new Book scenes. Scener keeps its native primitive local
axes; rotate a wall or cylinder's local Y height into world Z explicitly. See
[RENDERING.md](RENDERING.md) and validate cameras and exported anchors together.

Use both a human furniture reference and Pip's approved 8–12 cm height when
blocking Wondertown. A human-sized passage and a Pip route are different tests.
Keep scale references consistent through floor, tabletop and loft views.

Workshop architecture uses smooth Roman arches, plaster, timber and exposed
beams. Use `bool-negative-arch` for its arched openings; check the current
Scener scene-format guide for supported cutters. Do not substitute a rectangle
because an old process example used one. Other locations follow their own brief.

## Three passes with different detail budgets

### 1. Spatial blockout

Build the shell, actual openings, levels and four or five major furniture
masses. Include ceiling and beams. Start with anonymous boxes, then render a
perspective and `make layout ROOM=<room>` from Book. The layout command creates
an overhead diagnostic; it does not require a `LayoutPlan` story camera.

Check routes, scale, doorway alignment, foreground/middle/background and UI
crop before adding dressing. A roughly 10 cm minimum primary silhouette is a
useful workshop blockout shortcut, **not a finished-scene limit**. Small story
objects are exceptions, especially at Pip's scale. Proceed when the render and
plan pass these checks; no separate approval step is implied.

### 2. Designed room

Replace proxy masses with recognizable furniture and structural assemblies:
bench legs and apron, usable drawers, a shaped clock case, shelf supports,
window reveals, door thickness, loft support and a real ladder landing. Design
silhouettes first, then materials. Avoid a dial pasted onto a slab or shelves
that read as empty planks.

Populate the room by activity rather than scattering props. Give each major
work area a purposeful cluster: repair tools beside a partly repaired toy,
stored supplies on shelves, a tray and book beside a working surface. Keep
these clusters subordinate to the scene's focal subject. Non-interactive
objects may enrich the room without receiving ZIL names or hotspots.

### 3. Finished background

Add camera-visible secondary forms, material changes, fittings and selective
wear. Thin handles, clock hands, book spines and tool parts are appropriate when
they explain function or read in a focus view. Direct Scener backgrounds need
this design in geometry/materials; a later illustration pass must not be the
only plan for making an empty render appealing.

Use large, medium and small forms together. As an initial workshop composition
exercise, try 3–5 furniture masses, 2–4 purposeful dressing clusters and one
restrained foreground framing group. These are starting points, not quotas or
measured rules from the reference games. Judge projected coverage at delivery
size. A shelf of tiny indistinguishable cubes adds count without adding richness.

Vary shelf occupancy, height, spacing and orientation in supported groups. Give
one area a dense history of use and another a clean active surface. Put wear
where hands, feet and tools actually touch. Do not distribute contrast and
clutter evenly, fill every floor gap, or invent keys, documents or unusual
symbols that look like new clues.

## Compose a sequence of cameras

Write the shot's purpose, required visible subjects, approach/departure direction,
state and UI footprint before setting its camera. Explore three candidates from
the same unchanged room: an oblique wall/corner view, a lower occupant-scale view,
and a threshold/reverse view. Compare thumbnails, then refine the strongest.

For a high establishing view, 65–85% of room height, 15–35 degrees downward pitch
and 55–75 degrees **vertical** FOV are optional starting ranges. They are not a
requirement for every room. For a tiny protagonist, include a lower view that
lets furniture tower above the route. Human-height and low cameras can establish
geography too; clarity determines their role.

Move the camera and target before widening the lens to fit everything. Excessive
wide angle stretches foreground props and makes destinations tiny. Keep the
camera within plausible architecture and inspect beam/ceiling occlusion. Do not
disable a wall's shadows merely to rescue an obstructed camera. If a cutaway is
needed for a diagnostic view, keep it separate from production coverage and
check that production enclosure still blocks light correctly.

Compose three depth layers: a restrained edge in the foreground, the actionable
subject in the middle, and a destination or receding architecture behind it.
Two converging wall planes are a useful depth cue, not a compulsory formula.
Avoid equal-weight rows of furniture against a back wall. Leave distinct screen
regions around the identifying faces of required objects: ladder rungs, an
open shelf face, a workbench top, a clock dial. Check tangencies at frame edges
and where dark silhouettes overlap. Crop decorative framing boldly; keep the
identifying part of an actionable subject comfortably inside the runtime crop.

Each establishing shot is the interaction overview for its current location.
Every currently available subject must be discoverable there, together with
travel in at least three readable directions where the story permits it. Do not
require a hidden camera sweep to find an ordinary room object. Closely related
small parts, such as hook and string, can share a focus entry with explicit local
choices. Distinct physical zones can lead to their own establishing views.

Use either a corner showing two populated wall planes plus a back affordance,
or a camera against one wall showing the other three. A surface beneath a high
camera should contain objects whose tops communicate their purpose. Reserve a
clear back-arrow position in the presentation layer; never bake navigation into
the background JPEG. A back control returns to the parent view and must not
invent a parser exit. Stage hierarchy through size, spacing and light while
keeping every current room interaction available.

### Cuts and connected travel

Review a contact sheet in travel order: arrival → approach → threshold → next
arrival, then the reverse route. Preserve a recognizable landmark across each
cut, or supply a threshold view that restores orientation. Keep travel direction
consistent on screen where practical; if the camera crosses the movement axis,
use an explanatory doorway or neutral view instead of an unexplained reversal.

Match floor levels, doorway views, recurring objects and world-space light
sources across the cut. Screen-left illumination may become screen-right in a
reverse view; do not rotate the world light to force the same screen direction.
Change shot scale or angle for a meaningful action/reveal, while leaving a
stable establishing camera for returning to the same unchanged room state.

## Density, negative space and the Book overlay

Density is the distribution of readable forms, not a prop count. At thumbnail
size, identify one main subject, several supporting masses and a quiet reading
field. At runtime size, inspect materials, support and interaction silhouettes.
If the room reads as a showroom, add purposeful medium-scale clusters and
architectural layering before adding tiny debris. If it reads as noise, group
values and remove competing accents before deleting all dressing.

The workshop places prose lower-right and choices lower-left. Check the actual
UI for other shots. A 1536×1024 source fills a 1024×768 page with a centered
horizontal crop; inspect that crop, not just the uncropped source. Use real
prose, choice text and interaction circles. Quiet the reading field using a
broad material/value mass and soft light transitions. It can contain subdued
floorboards and shadows; it must not look like an artificially empty rectangle.
Do not move a required prop beneath prose or shorten story text to fix framing.

## Lighting and shadows

First establish a readable value composition with one motivated key source.
Add restrained fill to retain information in recesses, then a secondary practical
or doorway spill only where it explains the room. Warm lamps and cool moonlight
suit Wondertown, but a night shot may have a cool dominant key. Do not add warm
rim lights to every object by formula.

Put each practical source inside its fixture's local assembly. Check that the
bulb does not shadow its own emitter and that its opaque shade blocks light in
the intended directions. Window light must agree with the opening, reveal depth
and visible exterior. Preserve source positions and scene time across cameras.

Tune the key alone before adding fill. Inspect the shadows under furniture and
at wall/floor contacts: objects must feel supported. Cast shadows should describe
architecture and lead the eye, without producing a black lattice across a clue
or a harsh stripe through prose. Test long diagonal shadows as deliberately as
short ones. A 30–45 degree downward directional source is a useful starting
point, not a ban on low-angle sunlight or moonlight. Diagnose streaks by checking
source position, wall cuts, shadow-casting geometry and receiver surfaces.

Separate desired lighting from renderer support. Do not promise area-light
penumbrae, ambient occlusion, bounce lighting, textured materials or fog without
checking the current renderer. More point lights do not automatically produce
soft shadows; they can create multiple competing hard silhouettes. Use supported
fill carefully and log a concrete Scener limitation when the intended result
cannot be achieved. [RENDERING.md](RENDERING.md) describes that development loop.

Review in color and grayscale at delivery size. Keep a full value range while
retaining the route and story objects in shadow. Avoid clipped lamp pools,
uniform brown lighting, glossy highlights on every material, detached contact
shadows, light leaking through closed walls and bright shadows-as-subjects.

## Interaction anchors and state

Use the exact ZIL identifier for an interactive object's named group, with its
origin at the intended interaction center. Pure decoration gets distinct names
that do not impersonate story objects. An empty anchor is useful while blocking,
but it does not make the final pictured object visible: model or paint its
recognizable form and verify it against the projected marker.

Projection checks only the point against the camera; it does not prove that a
bench or wall is not hiding it. Inspect each required subject in the actual
render. Cropped or unmodelled subjects need the runtime's text-choice fallback.

Record visible state for every shot. Test present/removed props and closed/open
or folded/deployed mechanisms. A taken object must disappear from the image,
including its obsolete contact shadow. Permanent dressing stays put. Scener's
camera features are not a substitute for the host's ZIL state mapping.

## Spatial Plausibility and Functional Clearance

A blockout must describe a place that can actually be occupied and used. Good
framing does not excuse impossible architecture. Before adding detail or approving
a camera, mentally walk a person from every entrance to every exit, workstation,
vertical transition, and required story object.

Use these rules for every room layout:

1. **Preserve circulation.** Keep a continuous route wide enough for the intended occupant between all
   entrances, exits, and required destinations. Do not make a route depend on
   squeezing through furniture, stepping over props, or clipping a wall.
2. **Keep transition zones empty.** Do not place furniture, crates, shelves, or
   decoration in doorways, arches, corridors, stair landings, or the approach to
   a ladder. Include room for door swing where a hinged door is implied.
3. **Resolve vertical transitions.** Every ladder and stair must connect two usable
   surfaces. It must not terminate beneath a ceiling, beam, shelf, wall, or solid
   floor slab. Provide a visible hatch or landing, head and shoulder clearance
   through it, and enough clear floor to mount, dismount, and turn safely.
4. **Make climbable geometry read as climbable.** When a ladder's function matters,
   block rails and regularly spaced rungs rather than using one solid vertical box.
   Align its top with the hatch or landing and support its bottom on the floor.
5. **Protect windows.** Keep shelves, cabinets, picture frames, crates, and tall
   furniture out of window openings, shutter paths, and principal light paths.
   An obstruction is allowed only when the story explicitly requires it.
6. **Give furniture a use zone.** Leave an accessible working side at desks and
   benches, pull-out space behind chairs, opening space in front of drawers and
   cabinets, and a reachable face on shelves and storage.
7. **Mount wall objects plausibly.** Picture frames, clocks, hooks, and shelves sit
   on wall surfaces, not inside them or across openings. Keep them separated from
   doors, windows, trim, and one another. Shelves need believable support, depth,
   and vertical room for contents.
8. **Prevent interpenetration and unsupported forms.** Furniture does not pass
   through walls or other furniture. Platforms, lofts, shelves, and heavy objects
   need visible structural support appropriate to the blockout level.
9. **Keep exits usable beyond the silhouette.** Check both sides of every opening.
   A clear doorway is still invalid when the destination side immediately collides
   with furniture, a wall, a drop, or another transition.
10. **Check occupant scale in all three axes.** Compare widths, heights, reach, headroom,
    and step distances against the intended character. A valid top-down footprint
    can still hide a head strike or unreachable surface.

Validate these rules in both representations. The layout render reveals footprints,
routes, and overlaps; perspective renders reveal head clearance, mounting height,
depth, support, and occlusion. Inspect each transition from its approach and landing,
and render every neighboring camera affected by a moved architectural element.

### Automated intersection sanity

Wrap the primitives belonging to each multi-part story object in one named `<group>`
whose name exactly matches its ZIL identifier. The group owns both the interaction
anchor and the object's geometry:

```xml
<group name="SWEEP-BROOM" pos="-215 390 90">
   <box rot="0 -10 0" size="12 12 180" material="wood_oak"/>
   <box pos="0 0 -82" size="70 24 16" material="wood_dark"/>
</group>
```

Run the semantic bounds check directly or through the normal render workflow:

```sh
make sanity ROOM=workshop
make measure ROOM=workshop OBJECT=SWEEP-BROOM
make measure ROOM=workshop OBJECT=SWEEP-BROOM TARGET=WORKBENCH
make render ROOM=workshop   # runs sanity first
make layout ROOM=workshop   # runs sanity first
```

The measurement command reports the transformed world bounds, clearances to the
west, east, south, and north interior wall faces, floor and ceiling clearance,
and nearest edge-to-edge distance to other semantic assemblies. Distances include
rotation. Use a measure-adjust-remeasure loop: place the object upright, inspect
its baseline clearances, apply the intended lean or rotation, then inspect the
new clearances before rendering. Do not assume a rotation produces a particular
offset; for example, a 180 cm handle leaned by 2 degrees moves its end only about
$180\sin(2^\circ) \approx 6.3$ cm across its full length, or about 3.1 cm from a
center pivot.

Joined primitives inside one named group may overlap because they form one assembly,
such as ladder rungs meeting rails or table legs meeting a top. The checker reports positive-volume AABB
overlap between separate named groups as an error; rotated or nested shapes may
need inspection because bounding boxes are conservative. Surface contact alone is allowed.

This is a focused guard, not a physics engine. It currently checks box geometry in
named groups. It does not validate ungrouped decoration, detailed mesh or prefab
surfaces, door swing, reachability, moving states, or human headroom. Continue to
inspect both layout and perspective renders for those conditions.

## Reference study: exported AITD scenes

Local exports are at `~/Developer/Temp/AITD/output/`; floor files include
`AITD1_floor00.xml` through `AITD1_floor04.xml`. Other export variants may exist
in `output2/`, `output3/` and `output4/`; record the exact variant used.

Treat these as camera and spatial reference data, not finished material or
lighting targets. The inspected `output/AITD1_floor00.xml` includes box proxies
and a generic scene light. Its coordinates appear to use Y as height and
metre-like dimensions, unlike Book's Z-up centimetres. Verify the converter's
axis, scale, handedness and FOV convention before importing a camera or measuring
it; do not paste those transforms into Book unchanged.

Choose a small related group of cameras and note framing, visible walls,
foreground obstruction, route, reveal and the landmark retained across each cut.
Pair proxy views with an identified original background when available before
judging density, texture or shadow quality. Record file, camera ID and screenshot
with each finding. The previously quoted 89-camera percentages had no retained
measurement method here; do not use them as verified design constraints.
Resident Evil is a qualitative staging reference until specific reference images
are supplied and logged, not a source of measured lens presets.

## Acceptance and iteration

For each changed room, follow the commands in [RENDERING.md](RENDERING.md):
run sanity, render the layout and affected cameras, and inspect the results.
A plan cannot prove vertical clearance or shadow quality. Regenerate camera
metadata and matching state renders when their source changes.

Review these questions in order; fix the first failure before polishing detail:

1. **Story and continuity:** correct source locations, real connections, stable
   scale, matching door states and no invented clues?
2. **Use:** supported furniture, clear routes, reachable objects and viable
   climbs/landings for the intended occupant?
3. **Composition:** clear subject at thumbnail size, layered depth, readable
   functional faces and useful framing at the app crop?
4. **Richness:** designed architecture and furniture, purposeful dressing
   clusters and varied materials, with neither bare walls nor uniform clutter?
5. **Light:** motivated source, readable recesses, grounded contacts and useful
   shadows, without leaks or competing high-contrast patterns?
6. **Sequence and UI:** coherent forward/reverse travel, consistent state,
   readable prose/choices and correctly registered visible hotspots?

Save the plan, representative before/after renders and a contact sheet with the
room brief. Record remaining visual defects separately from renderer defects.
Do not call a scene finished from a successful CLI exit or numeric camera check.
