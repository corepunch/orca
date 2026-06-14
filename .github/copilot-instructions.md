# ORCA — Agent Startup Instructions

Keep this file small. Zed and other coding agents inject it into every new
agent session, so large reference material belongs in `.github/agent-context/`
and should be read only when relevant.

## First Rules

- Convention over configuration.
- Don't Repeat Yourself (DRY).
- ORCA is pre-release: prefer the cleanest architecture over backward
  compatibility. Remove obsolete APIs and style aliases instead of layering
  deprecations.
- For UI clipping, use `Overflow` (`overflow`, `overflow-x`, `overflow-y`) as
  the source of truth. Do not introduce or rely on `ClipChildren`.
- Do not preload large files or attach broad `@file` context unless the user
  explicitly asks. Prefer `rg` and targeted file reads.

## Concise C Style

- Minimize vertical space; prefer fewer, denser lines over many short ones.
- Single-statement functions go on one line: `int f(void) { return 0; }`
- Omit braces for single-statement `if`/`else`/`while` bodies.
- Chain closely related statements on one line with `;` when readable.
- Merge declarations that belong to the same logical step.
- Prefer aggregate assignments and shorthand initializers for structs, vectors,
  rectangles, boxes, and thickness values.
- Prefer helpers like `VEC2_*`, `VEC3_*`, `RECT_*`, and `BOX3_*` over manual
  member-by-member math.
- Use the comma operator inside ternary branches when it keeps conditional
  initialization clear and compact.
- Avoid helper variables that only name an already-readable expression.
- Do not split a declaration from its first assignment.

## Repository Map

```
orca/
├── source/          # C engine: core, renderer, filesystem, geometry, parsers
├── plugins/         # UIKit, SceneKit, SpriteKit, DarkReign, other plugins
├── libs/            # External libraries and platform submodule
├── include/         # Public C headers
├── docs/            # Architecture, guides, schemas, MkDocs site
├── tools/           # Code generation toolchain
├── tests/           # Lua and headless test harnesses
└── samples/         # Example projects
```

Important areas:

| Area | Start Here |
|---|---|
| Core object/component system | `source/core/`, `include/orca.h`, `include/orcadef.h` |
| UIKit | `plugins/UIKit/UIKit.cgen`, `plugins/UIKit/Button.c` |
| Lua bridge | `source/core/object/object_lua_*.c`, `share/orca/` |
| Code generation | `tools/codegen/`, `docs/MODULE_XML_GUIDE.md`, `docs/schemas/module.dtd` |
| Renderer | `source/renderer/r_main.c`, `source/renderer/r_draw.c` |
| Styling | `docs/styling.md` |
| Modal popup examples | `samples/Example/Screens/GetStartedPopup.xml` |
| Tests | `tests/test_styles_lua.lua`, `tests/test_state_manager.lua` |

## Build and Test

After cloning, initialize submodules:

```bash
git submodule update --init --recursive
```

Build:

```bash
make            # full build, requires system libraries
make unite      # incremental C build
make modules    # regenerate generated headers, properties, exports
```

Tests:

```bash
make test-headless
xvfb-run make test
```

Generated files under `generated/` are ignored. Do not hand-edit generated
outputs; change `.cgen` files and rerun code generation.

## Component and Codegen Workflow

When adding or changing a component, property, message, or Lua-facing API:

1. Edit the relevant `.cgen` file first.
2. Run `make modules`.
3. Implement or update C handlers using `HANDLER(CLASS, NS, EVENT)`.
4. Add/update tests.
5. Run the narrowest useful build/test command available.

Handlers without matching `<handle message="..."/>` entries are orphaned and
will never run. A generated Proc with an empty switch means the `.cgen` step is
incomplete.

Core must not include plugin headers such as `<UIKit/UIKit.h>`. Plugins may
depend on core; core must communicate with plugins through messages or generic
interfaces.

New `.c` files under `source/core/components/` or `plugins/*/` must be added to
`orca.xcodeproj/project.pbxproj` as well as the Makefile path.

## Key API Reminders

- Object/component handlers use `HANDLER(CLASS, NS, EVENT)`, not raw Proc
  signatures.
- `objectProc_t` receives `(struct Object *obj, void *cmp, uint32_t msg,
  wParam_t wParam, lParam_t lParam)`. `cmp` is the component data block.
- Switch cases for generated message IDs must mask with `MSG_DATA_MASK`.
- Generated accessors like `GetAnimationPlayer(obj)` return `NULL` when the
  component is not attached.
- Lua can dispatch object messages directly with capitalized method names, for
  example `screen:SetModalObject(modal)`.
- Inline XML can use WPF-style message shorthand such as `{Node.RightButtonUp}`,
  `{Popup.ClosePopup}`, or `{Screen.ShowModal Path=...}`.

## Popup Convention

- Modal popups should be `Popup` objects, not `Screen` placeholders.
- `{Screen.ShowModal Path=...}` points to the popup template asset.
- Popups usually rely on a full-screen background/overlay and should not set
  root `Width`/`Height` unless there is a specific layout need.
- Use `Popup.ClosePopup` to dismiss the popup and return its result.

## Area Instructions

Before changing code, read only the area file that matches the task. Read more
than one only when the task crosses boundaries.

| Task | Read |
|---|---|
| UI elements, UIKit controls, XML screens, styles, popups, layout tests | `.github/agent-context/ui-elements-and-styling.md` |
| Lua APIs, bindings, property formulas, scripts, app framework behavior | `.github/agent-context/lua-bindings-and-programs.md` |
| Core object/component runtime, filesystem, parsers, localization, network | `.github/agent-context/core-object-and-filesystem.md` |
| `.cgen` schema, generated headers/exports/properties, new C components | `.github/agent-context/codegen-and-components.md` |
| Renderer, textures, palettes, shaders, meshes, WebGL, DarkReign assets | `.github/agent-context/renderer-and-assets.md` |
| Build system, tests, Xcode project wiring, docs site | `.github/agent-context/build-tests-and-docs.md` |

The full historical guide remains at `.github/agent-context/orca-reference.md`
for rare deep dives. Do not read it by default.

## Local-Agent Performance Hygiene

- Do not attach large files with `@file` when the agent can read them on demand.
- Avoid duplicating the same prose and file mention in a single prompt.
- Keep new always-on instruction files short; move examples, tables, and gotchas
  into lazy-loaded docs under `.github/agent-context/` or `docs/`.
