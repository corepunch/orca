# Project Map

Use this file to find the right subsystem before reading source.

## Core paths

- `source/core/` — object runtime, components, property system, Lua bridge entry points
- `source/filesystem/` — package loading, XML/Lisp object loading, asset lookup
- `source/parsers/` — parser helpers and XML support
- `source/renderer/` — renderer, textures, meshes, shaders
- `source/geometry/` — vectors, matrices, math types
- `plugins/UIKit/` — UI controls, layout, popups, style-adjacent behavior
- `plugins/SceneKit/` — 3D scene graph
- `plugins/SpriteKit/` — 2D sprite graph
- `plugins/EditorKit/` — editor serialization and editor-specific helpers
- `tests/` — C tests and Lua tests
- `tools/codegen/` — `.cgen` loader and generator plugins
- `docs/` — human and agent-facing project maps, guides, and architecture notes

## Source-of-truth files

- Public API shape: module `.cgen` files in `source/**` and `plugins/**`
- Build and test commands: `Makefile`
- MkDocs navigation: `mkdocs.yml`
- Generated outputs: `generated/**` (never edit by hand)

## Fast routing

- Add/change Lua-visible API: `docs/LUA_API.md`
- Add/change a component: `docs/ADDING_CLASSES.md`
- UI or layout behavior: `docs/UI_SYSTEM.md`
- Test work: `docs/TESTING.md`
- Build failures or command choice: `docs/BUILD.md`
- High-level runtime model: `docs/ARCHITECTURE.md`

## Dependency boundaries

- Core may not include plugin headers.
- Plugins may depend on core.
- Shared behavior across modules should flow through messages, properties, or generic interfaces.
