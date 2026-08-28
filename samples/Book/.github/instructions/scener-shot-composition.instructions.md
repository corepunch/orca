---
name: "Scener Shot Composition"
description: "Use when building or revising Scener scenes, blockouts, establishing shots, and camera layouts in .blk or .blks files."
applyTo: "Rooms/**/*.blk, Rooms/**/*.blks"
---
# Scener Shot Composition

- Follow `Work/SCENE_COMPOSITION.md` for the complete rationale and review workflow.
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
- Never place objects or furniture directly in front of doors, window cutouts, or
  passages that obscure their silhouettes or light paths. All openings must have
  an unobstructed line of sight and passage.
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
- Use a frontal camera only when a specific story beat requires symmetry or
  direct confrontation.
- Validate the unstyled blockout render. If the shot is boring without surface
  treatment, improve the camera and spatial staging before proceeding.