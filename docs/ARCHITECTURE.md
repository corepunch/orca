# Architecture

Use this file for the smallest useful mental model of ORCA.

## Runtime model

ORCA is an object/component engine.
`Object` is the universal scene node; behavior is attached through components declared in `.cgen` and implemented in C.
Messages drive most cross-system behavior.

## Main layers

- **Core runtime** — object lifecycle, component dispatch, property storage, Lua bridge glue
- **Filesystem and parsers** — package loading, XML/Lisp loading, asset discovery
- **Renderer** — drawing, textures, meshes, shaders, platform graphics integration
- **Plugins** — UIKit, SceneKit, SpriteKit, EditorKit, and others built on top of core
- **Lua surface** — generated module exports plus handwritten runtime scripts in `share/orca/`
- **Codegen** — `.cgen` files generate headers, property hashes, and Lua exports

## Architectural rules

- `.cgen` is the source of truth for public API shape.
- Generated files under `generated/` are outputs, not edit points.
- Core must remain buildable without plugin header dependencies.
- New behavior should usually be a component, not a new embedded field on `Object`.
- Cross-subsystem calls should prefer messages over tight direct coupling.

## Read order by task

- Need the object model: `docs/architecture/object-component-system.md`
- Need codegen details: `docs/architecture/code-generation.md`
- Need plugin boundaries: `docs/architecture/plugin-system.md`
- Need XML message shorthand: `docs/app/xml/triggers-actions-messages.md`

## For local models

1. Read this file.
2. Read `docs/PROJECT_MAP.md` if you need paths.
3. Read one deeper document only for the subsystem you are changing.
