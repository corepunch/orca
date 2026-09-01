---
name: "Scener Shot Composition"
description: "Use when building or revising Scener scenes, blockouts, establishing shots, and camera layouts in .blk or .blks files."
applyTo: "Rooms/**/*.blk, Rooms/**/*.blks"
---
# Scener Shot Composition

- Follow `Work/SCENE_COMPOSITION.md` for the complete rationale and review workflow.
- Name every story camera after the ZIL room or object it depicts, converted to
  lowercase while preserving hyphens: `WORKSHOP-FLOOR` becomes `workshop-floor`
  and `LOFT-LADDER` becomes `loft-ladder`. Do not use CamelCase or append camera
  roles such as `Focus`, `Closeup`, or `Establishing`; Scener uses the camera name
  as the screenshot filename. `layout.jpg` is the sole diagnostic exception.
- Wrap the visible primitives of each multi-part story object in one named `<group>`
  using its exact ZIL identifier. Geometry inside one group may join or overlap as
  part of the same assembly; separate named groups must not intersect. Run
  `make sanity ROOM=<room>` before visual review. Render and layout targets run
  this check automatically and must not proceed when semantic objects overlap.
- Use measurement rather than visual guessing when placing objects near walls,
  furniture, openings, or each other. Run `make measure ROOM=<room> OBJECT=<ZIL-ID>`
  for all room-shell and semantic-object clearances, or add `TARGET=<ZIL-ID>` for
  one pair. Place the object unrotated, measure, apply the intended lean/rotation,
  then measure again because rotation changes its world-space extents. Keep small
  intentional clearances as test assertions when later edits could regress them.
- Use the original 1992 *Alone in the Dark* as the primary fixed-camera
  reference. Favor sparse, readable silhouettes and deliberately restricted
  views over the denser environmental detail of later survival-horror games.
- Compose the shot during geometry blocking, before applying art style,
  materials, lighting polish, or painted detail.
- Never default to a flat, straight-on room elevation for an establishing shot.
- Default establishing cameras to a room corner below the ceiling, aimed
  diagonally across the room. Start at 65-85% of the room height, pitch down
  15-35 degrees, and use a 55-75 degree FOV. Keep the camera below beams and
  other ceiling structure that would obscure the view.
- Reserve floor-level or centered-room cameras for intentional extreme
  perspective, suspense, or action shots; do not use them as the establishing
  default.
- Place a meaningful room corner near the central third of the frame so two
  wall planes and converging perspective lines are visible.
- Build clear foreground, middle-ground, and background layers with overlap.
- Ensure key objects, workstations, and major exits are prominently and clearly
  visible in establishing shots. Never place primary interactable objects or
  workstations along a clipped foreground wall or beneath the camera where they
  are heavily cropped. Align major workstations along visible flanking walls
  (e.g., the left wall in a corner establishing shot).
- Every semantic element intended to communicate the room layout must be clearly
  readable in the establishing shot, not merely technically inside the frame.
  Show its complete identifying silhouette with comfortable margin from the frame
  edge; do not accept an element that is tiny, heavily occluded, edge-on, cropped,
  or reduced to an ambiguous fragment.
- Orient each important object toward the camera angle that best communicates its
  function. Prefer the view with the strongest recognizable silhouette: show a
  ladder from the rung side, a chair from an angle that separates seat and back,
  shelving from its open face, and a desk from a view that reveals its work surface.
  Rotate or relocate the object, or revise the camera, when its functional face is
  hidden. Physical placement and usability still take priority over presentation.
- Never place objects or furniture directly in front of doors, window cutouts, or
  passages that obscure their silhouettes or light paths. All openings must have
  an unobstructed line of sight and passage.
- Treat every layout as a physically usable place, not only a camera composition.
  Mentally walk a person from each entrance to every exit, workstation, stair,
  ladder, and story object. Preserve continuous standing-width routes and enough
  space to approach, turn, operate, sit at, or open each feature.
- Never terminate a ladder or stair into a ceiling, floor slab, beam, shelf, or
  wall. Provide a visible landing or hatch at least as wide as the climber,
  preserve head and shoulder clearance through the opening, and leave space to
  step off safely at both ends. Use rails and rungs instead of a solid post when
  the climb route needs to read clearly.
- Keep doors, arches, corridors, stairs, ladders, and transition zones free of
  furniture and props. Account for door swing, approach space, and the destination
  beyond an opening; a visible doorway is not usable if its route is blocked.
- Keep windows functionally clear. Do not place shelves, cabinets, picture frames,
  crates, or tall furniture across window openings, sills, shutters, or primary
  light paths unless the obstruction is an explicit story condition.
- Place wall-mounted objects on actual wall surfaces with plausible support and
  spacing. Do not overlap picture frames, shelves, hooks, clocks, windows, doors,
  trim, or each other. Shelves need usable depth and vertical space for contents.
- Keep furniture use zones plausible: chairs need pull-out space, desks and benches
  need an accessible working side, drawers and cabinet doors need opening clearance,
  and storage must be reachable without crossing another object.
- Check vertical as well as plan clearance. A top-down layout can miss head impacts,
  blocked hatches, low beams, floating objects, unsupported platforms, and objects
  embedded in walls or ceilings.
- Do not rely on the sanity check alone. It currently validates positive-volume
  overlap between box geometry in named semantic groups; it cannot understand
  ungrouped decoration, mesh/prefab surfaces, intended movement, or human clearance.
- Avoid duplicate or conflicting major exits; ensure distinct room functions and
  clear passage silhouettes.
- Keep directional and sunlight pitched downwards at 30 to 45 degrees from
  horizontal. Never use near-horizontal lighting (e.g. 10-20 degrees), which
  creates unnatural room-spanning light streaks.
- When story prose overlays the render, reserve one lower quadrant as deliberate
  text-safe negative space. Keep major props, exits, hotspots, and high-contrast
  detail out of that area.
- Include only objects whose primary silhouette is at least 10 cm in the scene's
  centimeter units. Omit smaller decorative components such as clock hands,
  knobs, handles, fasteners, loose debris, and surface clutter unless they are
  required for interaction or shot comprehension.
- Block large semantic forms such as desks, ladders, drawers, doors, windows,
  shelves, stairs, and major props. The blockout communicates stable layout; it must not imply
  that an artist should trace incidental detail literally.
- Maintain pure blocking discipline: avoid adding small decorative components (legs, handles,
  knobs, small clutter) during layout iteration. Rely on bold, well-proportioned masses,
  architectural framing (beams, mezzanines, pillars), and clear functional zones.
- Iterate through visual review: author blockout masses in `.blks`, render the establishing
  shot, critically review depth, overlap, silhouette readability, and spatial balance,
  and reposition masses until the shot reads strongly.
- Validate spatial logic with both `make layout ROOM=<room>` and perspective renders
  from every affected camera. Inspect the route, both endpoints, and neighboring
  geometry; never infer usability from an unobstructed anchor or one attractive view.
- Review the establishing render at final delivery size. For every required element,
  name what it is from silhouette alone and confirm that its entire meaningful form
  has clear frame margin. If an element is recognizable only because its location is
  already known, reorient, reposition, relight, or reframe it.
- Use a frontal camera only when a specific story beat requires symmetry or
  direct confrontation.
- Validate the unstyled blockout render. If the shot is boring without surface
  treatment, improve the camera and spatial staging before proceeding.