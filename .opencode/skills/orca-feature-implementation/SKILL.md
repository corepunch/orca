---
name: orca-feature-implementation
description: Implement an ORCA feature spanning one or more engine, plugin, UI, renderer, XML, or Lua subsystems. Use for behavior changes that require architecture routing, handwritten implementation, generated API decisions, build wiring, and focused verification rather than a single isolated API declaration.
---

# ORCA Feature Implementation

Route the change to the owning subsystem before reading implementation files.
Keep one coherent owner for the feature and delegate only independent parts.

## Workflow

1. Read `docs/PROJECT_MAP.md`, then the one subsystem document named there.
2. Identify the owning module and split the request into API shape,
   handwritten behavior, Lua exposure, XML authoring, assets/scenes,
   controller binding, and tests.
3. Read the relevant generated API reference under
   `.opencode/generated/{c,lua,xml}/<module>/`. Fall back to the owning `.cgen`
   only when the generated reference is unavailable.
4. Read one nearest implementation and one focused test at most.
5. State the smallest change that preserves subsystem boundaries.
6. Use `$orca-api-codegen` before implementation if public API shape changes.
7. Implement behavior in handwritten sources and add build/Xcode wiring for
   any new C file by following its nearest peer.
8. Use `$orca-lua-exposure`, `$orca-xml-authoring`,
   `$orca-xml-assets-scenes`, or `$orca-controller-binding` for those surfaces.
9. Add focused coverage with `$orca-testing`.
10. Run generation when needed, then build and test the narrowest touched
    surface. Broaden only when shared behavior changes.

## Architecture rules

- Add reusable behavior as a component instead of growing `Object`.
- Keep core free of plugin-header dependencies.
- Communicate across modules with messages, properties, or generic interfaces.
- Keep XML/package declarations declarative and runtime behavior in C or Lua.
- Remove the superseded singleton or global access path when ownership moves
  to a component; do not leave two authorities.
- Avoid unrelated cleanup and refactoring.

## Verification

Use `docs/BUILD.md` and `docs/TESTING.md` to select commands. Prefer:

```bash
make modules        # after .cgen changes
make unite          # after core or plugin C changes
make test-headless  # shared C, Lua, XML, or UI behavior
```

Use `xvfb-run make test` only for display-backed behavior.
