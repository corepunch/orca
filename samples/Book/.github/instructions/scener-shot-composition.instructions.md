---
name: "Scener Shot Composition"
description: "Use when building or revising Scener scenes, blockouts, establishing shots, and camera layouts in .blk or .blks files."
applyTo: "Rooms/**/*.blk, Rooms/**/*.blks"
---
# Scener Shot Composition

Read `AGENTS.md` for Book ownership and task routing. Follow the canonical
`Work/LOCATION_BRIEFS.md`, `Work/SCENE_COMPOSITION.md` and `Work/RENDERING.md`.
Do not maintain a second composition rule set here.

- Build connected physical zones from actual story source; parser exits do not
  automatically imply doorways. Keep geometry, states and world light coherent.
- Separate sparse spatial blockout from designed, populated final backgrounds.
  The early 10 cm proxy shortcut does not limit finished props or Pip-scale shots.
- Compare cameras at different heights and thresholds; select by narrative
  readability. Numerical high-corner presets are starting points, not gates.
- Validate support, routes, functional silhouettes, density, motivated shadows,
  UI crop and adjacent shots using the guide's review sequence.
- Use exact ZIL IDs for interactive groups and existing lowercase hyphenated
  IDs for runtime cameras. Check the presentation manifest before renaming.
- Run focused sanity/measurement and visual checks. Use the deployed Scener
  documented in RENDERING and verify at final raster size, including shadows
  across several camera positions.
