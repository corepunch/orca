# Architecture Overview

This section describes the internal architecture of the ORCA engine for developers who want to contribute to the engine, write plugins, or deeply integrate with it.

---

## Repository Setup

After cloning, initialize submodules before building or running code generation:

```bash
git submodule update --init --recursive
```

This pulls the required platform submodule:

| Submodule | Path | Purpose |
|---|---|---|
| `platform` | `libs/platform` | Cross-platform abstraction layer (window, input, audio) |

---

## Repository Layout

```
orca/
├── source/          # C engine source
│   ├── core/        # Object system, components, properties, events
│   ├── renderer/    # OpenGL rendering (shaders, textures, fonts, models)
│   ├── filesystem/  # Virtual file system, asset packages
│   ├── geometry/    # Math library (vec3, mat4, quaternions)
│   ├── parsers/     # XML and data parsers
│   ├── localization/# Multi-language string support
│   ├── network/     # UDP/TCP networking
│   └── editor/      # Built-in editor functionality
├── plugins/         # Plugin modules: UIKit/, SceneKit/, SpriteKit/, DarkReign/
├── libs/            # External libraries (platform submodule, lua5.4, libxml2, …)
├── include/         # Public C headers (orca.h, renderer.h, …)
├── docs/            # MkDocs website source (Markdown + schemas)
├── tools/           # Code-generation toolchain
│   ├── codegen/     # C codegen host and generator plugins
│   └── Makefile     # Module generation recipes used by root `make modules`
└── samples/         # Example projects
```

---

## Module Structure

Every C module lives in `source/<module>/` or `plugins/<name>/` and follows the same layout:

```
<module>/
├── <module>.cgen           # API definition — source of truth for the public interface
├── <module>.h              # Generated C header  (do not edit by hand)
├── <module>_properties.h   # Generated property hash constants  (do not edit)
├── <module>_export.c       # Generated Lua binding code  (do not edit)
└── *.c / *.h               # Hand-written implementation files
```

### Core modules (`source/`)

| Module | Purpose |
|---|---|
| **core** | Object system, component attachment, properties, events, animations, state machines, Lua scripting host |
| **renderer** | OpenGL rendering: shaders, textures, fonts, models, shapes, cinematic |
| **filesystem** | Virtual filesystem, asset package loading (.pz2), file server |
| **geometry** | Math types: `vec2`, `vec3`, `mat4`, `quaternion`, `box3` |
| **parsers** | XML parsing, binary serialization |
| **localization** | Multi-language string tables |
| **network** | UDP/TCP sockets |
| **editor** | In-engine editor tools |

### Plugin modules (`plugins/`)

| Plugin | Purpose |
|---|---|
| **UIKit** | Full UI framework — buttons, layouts, text, image views, forms, grids |
| **SceneKit** | 3D scene management, cameras, lights, mesh nodes |
| **SpriteKit** | 2D sprite and animation framework |
| **DarkReign** | Dark Reign game format support (.spr sprites, .pz2 packages) |
| **vsomeip** | AUTOSAR SOME/IP communication for embedded targets |

Each plugin module is compiled into its own shared library (`.so` / `.dll`) and loaded at runtime.

---

## Further Reading

- [Object + Component System](object-component-system.md) — How Objects and Components work, and how to add new component types.
- [Object Typedata Model](object-typedata-model.md) — Planned direction for replacing linked component storage with monolithic typedata families.
- [Orca Language](orca-language.md) — Living design note for replacing duplicated XML, C, and MoonScript declarations with `.orca` object definitions.
- [Atomic Properties And Authoring Shorthands](atomic-properties-and-shorthands.md) — Design draft for keeping generated structs flat while lowering grouped authoring syntax to atomic property writes.
- [Plugin System](plugin-system.md) — C component plugins and Lua script plugins, with UIKit as a worked example.
- [Code Generation](code-generation.md) — How the C codegen toolchain converts `.cgen` module definitions into C headers and Lua bindings.
- [Module Codegen Guide](../MODULE_XML_GUIDE.md) — Full reference for writing `.cgen` module files.
