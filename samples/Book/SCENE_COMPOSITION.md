# Scene and Camera Composition

This guide defines how Book scenes should be blocked for fixed-camera renders.
The primary reference is the original 1992 *Alone in the Dark*. The goal is not
to copy individual shots, but to use the same economical spatial language:
strong architecture, deliberate viewpoints, readable silhouettes, and limited
information.

## Where to Start

Begin with the room shell and four or five large anonymous boxes — one per major
furniture mass. For the workshop establishing shot that means:

1. **Room shell first.** Four walls, floor, ceiling. Add one door opening in the
   back wall so depth reads. Nothing else.
2. **One box per mass.** Workbench (left, mid-depth), tool counter (right, rear),
   loft platform (rear-left, elevated), crate-and-stool stack (right, mid).
   Use round numbers for size and position; exact fit comes later.
3. **Place two cameras and render both immediately.** The first two cameras to
   produce are always:
   - `WorkshopEstablishing` — the high corner view, two walls converging, full
     foreground/mid/background read. Follow the recipe in
     [Establishing Camera Recipe](#establishing-camera-recipe).
   - `LayoutPlan` — a straight-down view from just below the ceiling
     (`y ≈ 620`), FOV 92, aimed at the floor centre. Use a dark floor material
     (`floor`) and warm wood for furniture so the top-down plan is legible as a
     geography map.
   Render both before touching anything else. The plan catches positional errors
   that the perspective shot hides.
4. **Evaluate composition, not content.** On the establishing shot: are
   foreground, mid-ground, and background distinct? Does the loft break the
   ceiling line without dominating the frame? Is the text-safe lower-right floor
   clear? On the plan: do the furniture footprints leave clear routes to the
   exit?
5. **Stop.** Do not add prefabs, surface detail, secondary objects, or
   additional cameras until both renders read correctly.

The current `Rooms/workshop.blks` is at this stage. It has only the shell and
four masses. Render both cameras and confirm the layout before proceeding.

## What the Blockout Owns

The `.blks` scene is the shared spatial source of truth. It owns:

- room dimensions, walls, floor, ceiling, and major openings;
- stable locations and scale relationships for large objects;
- routes between entrances, exits, and interactive areas;
- named cameras and interaction anchors;
- the perspective, overlap, and negative space of each shot;
- broad light direction and pools of light.

The blockout does not own surface texture, ornament, wear, small hardware, or
literal final-art shape design. An artist should preserve its geography and
silhouettes, not trace every primitive.

## Coordinate Convention

Scene files use the 3ds Max axis convention. There is no other.

| Axis | Direction |
|------|-----------|
| X | East — right on any north-up map |
| Y | North — depth into the scene; Y=0 is the south wall |
| Z | Up — height above the floor; Z=0 is the floor |

Distances are centimetres. Rotations are degrees. `scale` is unitless.

## Geometry Budget

Model large semantic forms first: desks, benches, ladders, drawers, doors,
windows, shelves, stairs, and major story props. Each form should explain the
room or support an interaction.

Use 10 cm as the default minimum primary silhouette. Ignore smaller decorative
components such as clock hands, knobs, handles, fasteners, loose debris, and
surface clutter. A thinner part may remain when its overall silhouette is large
and necessary, such as a ladder rail, shelf board, or book.

Interaction-critical objects are exceptions to the size threshold. Keep them
simple, named, and visually distinct rather than surrounding them with detail.

Do not add small boxes merely to make a room look populated. They create false
continuity obligations and imply that final art should reproduce arbitrary
blockout noise.

## Room Structure

Build complete rooms rather than wall-and-floor stages. Include the ceiling and
major beams so a camera can be placed against real architecture and checked for
occlusion.

For a standard establishing shot:

- expose two wall planes and their intersection;
- place at most one visible exit on each wall;
- imply the entry behind or beside the camera when possible;
- keep clear routes between exits and interactive landmarks;
- distribute major objects across foreground, middle ground, and background;
- avoid putting every landmark against one back wall.

The room may contain more narrative destinations than one image shows. A fixed
camera should deliberately restrict information rather than flatten the entire
map into one diagram.

## AITD1 Camera Findings

The converted original scenes in `AitD/output/AITD1_floor00.xml` through
`AITD1_floor04.xml` contain 89 mansion cameras. Comparing each camera with the
nearest room wall bounds gives this baseline:

| Measurement | Result |
|---|---:|
| Cameras near at least one wall | 88% |
| Cameras near a room corner | 40% |
| Cameras looking diagonally to wall axes | 66% |
| Median height relative to room height | 70% |
| Median downward pitch | 26.7 degrees |
| Median field of view | 55.3 degrees |

The original game varies its cameras substantially. These numbers do not mean
that every AITD shot is a high corner view. They show that cameras are usually
architecturally anchored, commonly elevated and downward-looking, and often
oblique. Low and frontal cameras are deliberate dramatic exceptions.

## Establishing Camera Recipe

Use this starting point, then compose against the actual room:

1. Place the camera near a room corner, not in the middle of the room.
2. Start at 65-85% of room height, below ceiling beams and rafters.
3. Aim diagonally across the room so two walls converge in frame.
4. Start with 15-35 degrees of downward pitch.
5. Start with a 55-75 degree FOV.
6. Put an exit or key landmark near the vanishing direction.
7. Establish foreground, middle-ground, and background overlap.
8. Reserve a clear region for prose before adding secondary geometry.

Do not treat the numeric ranges as a substitute for looking at the render. A
camera can satisfy every number and still be blocked by a beam or furniture.

Use floor-level, centered, straight-on, or extreme overhead cameras only when a
specific beat needs intimidation, concealment, disorientation, symmetry, or a
map-like view. Name that purpose in the camera comment.

## Shot Roles

Each production camera should have one narrative job:

- **Establishing:** communicates geography, routes, and a few affordances.
- **Focus:** isolates an object or conversational subject.
- **Action:** emphasizes movement, scale, or mechanical cause and effect.
- **Reveal:** changes what the viewer understands about a known location.
- **Plan:** documents continuity and is not production story art.

Do not reuse one neutral camera for every state. Alternate wide, medium, action,
and close views while keeping object locations and light direction stable.

## Text-Safe Negative Space

When prose overlays the image, negative space is functional composition, not an
unfinished area. Reserve one lower quadrant and keep it free of:

- major props and exits;
- projected interaction hotspots;
- strong silhouette boundaries;
- high-frequency detail;
- bright highlights that reduce text contrast.

For `WorkshopEstablishing`, the lower-right floor is the text-safe area. Story
prose is aligned there, choices remain lower-left, and Continue sits below the
prose. Other cameras may use a different quadrant, but the scene and UI must
agree on it.

## Occlusion and Landmark Separation

Projection only proves that an anchor lies inside the camera frustum. It does
not prove that the object is visible. Large foreground geometry can cover a
valid hotspot completely.

Separate important landmarks in both world space and screen space. In the
workshop, the tool counter, rack, and display case originally shared the same
sightline as the frosted window. Moving the window along the wall fixed the wide
shot, but the countertop camera still placed the case in front of it. The final
solution also moved the countertop camera so the window, case, and doll occupy
different screen regions.

After changing a camera or large object:

1. Project every required interaction anchor.
2. Render the changed camera.
3. Inspect silhouettes and occlusion visually.
4. Check the text-safe area at the runtime crop and aspect ratio.
5. Render neighboring cameras that share the moved object.

## Lighting

Use one or two motivated light pools instead of uniform ambient illumination.
Light should identify an exit, route, or interactive landmark while leaving
quieter areas dark enough to create depth.

Keep the text-safe region comparatively even and low-detail. A soft value shift
is useful; a point-light hotspot directly behind prose is not.

## Naming and Continuity

Name cameras by narrative function and name groups used as interaction anchors.
Camera comments should state placement, target, and any reserved text region.

Keep all shots in the same scene coordinate system. Move a shared object once,
then review every camera that sees it. Do not repair continuity by moving the
same landmark independently between rendered shots.

After moving cameras or named anchors, regenerate metadata:

```sh
lua Tools/export_workshop_camera.lua
```

Then run the focused checks:

```sh
lua Tests/test_scene_projection.lua
lua Tests/test_camera_export.lua
cd libs/zilscript && lua ../../Tests/test_workshop_prototype.lua
```

Finally, render and inspect every changed camera. The exported metadata and JPEG
must come from the same `.blks` revision.

## Review Checklist

- Camera is attached to a wall or corner rather than floating centrally.
- Establishing view shows two converging wall planes.
- Ceiling geometry does not dominate or block the view.
- Major landmarks have distinct silhouettes and screen regions.
- Foreground, middle ground, and background are readable.
- Exit or key object receives compositional emphasis.
- Text-safe negative space is intentional and matches the UI overlay.
- No incidental object under 10 cm creates a false art requirement.
- Required anchors project inside the frame.
- The render has been inspected for real occlusion.
- Neighboring shots preserve layout, object state, and light direction.
