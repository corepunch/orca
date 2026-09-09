# Workshop Layout Notes

Use with [LOCATION_BRIEFS.md](LOCATION_BRIEFS.md) and
[SCENE_COMPOSITION.md](SCENE_COMPOSITION.md). Earlier notes described a sparse
snapshot and proposed exact replacement coordinates; those are not current
measurements. Inspect `Rooms/workshop.blks`, its layout and relevant cameras
before deciding what still needs work.

## Source corrections to retain

- `WORKSHOP-FLOOR`, `WORKBENCH-TOP`, `TOOL-BENCH`, `COUNTERTOP` and `STORAGE-LOFT`
  are gameplay locations within one workshop volume. An EAST move to TOOL-BENCH
  does not require an east-wall doorway.
- NORTH leads through the pet door to SNOWY-ALLEY. The pet door is in the bottom
  of the main workshop door. Match its interior and exterior placement.
- UP to STORAGE-LOFT depends on LADDER-OILED. Include the folding LOFT-LADDER
  and rusted iron LADDER-MECH, with a supported landing and viable deployed state.
- The conditional IN connection to TOLLIVER-STUDY depends on STUDY-ACCESS.
  Resolve its stairs/elevation against source before adding architecture.
- Use exact IDs: SWEEP-BROOM, KEY-HOOK, KEY-STRING, CLOCK-FACE, OIL-CAN.
  `BROOM` was an imprecise label in earlier notes.
- Keep OIL-CAN beneath the workbench in shadow, with a revealing focus view.
- Separate stored crates from MAKESHIFT-STEPS. The low crate, chair and three
  repair books make a functional climb to COUNTERTOP; pushing every crate
  against a wall would destroy that route.

## Next visual pass

1. Render the actual plan and establishing shot. Check the source-backed route
   from floor to tool bench and climb, plus both sides of the pet-door threshold.
2. Compare required prop silhouettes at the runtime crop. An empty anchor is
   not a visible hook, clock, ladder mechanism or oil can.
3. Design workbench/clock/shelf forms beyond anonymous boxes. Add tools,
   shavings and unfinished toys in purposeful supported groups; keep the working
   face, climb and reading field usable.
4. Make the window a real opening with consistent frame/reveal proportions.
   Use workshop Roman arches and centimetres; the former 2.0 × 1.7 window
   example was not a usable full-size Book dimension.
5. Check light direction through the window and door, furniture contact shadows,
   lamp hotspots and dark-on-dark object separation. Review lower focus cameras
   alongside the high establishing view so Pip's scale remains apparent.
6. Render the affected travel sequence in both directions. Record measured
   defects and intended fixes in the current brief rather than replaying old
   position tables.

Use `make measure ROOM=workshop OBJECT=<ID>` for transformed bounds and
clearances, then render to confirm the physical and visual result. Numeric
separation alone cannot establish a convincing composition or a reachable climb.
