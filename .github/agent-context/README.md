# ORCA Agent Context

These files split the old long `.github/copilot-instructions.md` into focused,
lazy-loaded areas. Agents should read the smallest relevant file before editing
that area, instead of loading the full project guide on every session.

## Routing

| Task | File |
|---|---|
| UI elements, UIKit controls, XML screens, styles, popups, layout tests | `ui-elements-and-styling.md` |
| Lua APIs, bindings, property formulas, scripts, app framework behavior | `lua-bindings-and-programs.md` |
| Core object/component runtime, filesystem, parsers, localization, network | `core-object-and-filesystem.md` |
| `.cgen` schema, generated headers/exports/properties, new C components | `codegen-and-components.md` |
| Renderer, textures, palettes, shaders, meshes, WebGL, DarkReign assets | `renderer-and-assets.md` |
| Build system, tests, Xcode project wiring, docs site | `build-tests-and-docs.md` |

`orca-reference.md` is the archived long guide. Use it only when the focused
files and nearby source/docs do not contain enough context.
