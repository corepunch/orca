# ORCA — Copilot Instructions

This file gives GitHub Copilot (and any AI coding assistant) the essential context needed to work effectively in the ORCA repository without having to re-discover it from scratch.

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

## Building the Project

**Full build** (requires submodules + system libraries):
```bash
make
```
This runs code generation and then C compilation. System libraries such as lua5.4-dev, libxml2-dev, liblz4-dev, and friends must be present.

**Incremental C build**:

Generated files are written under `generated/` and are intentionally ignored by git. `make` and `make unite` both run `make modules` first, so the generated headers and exports are refreshed before compilation:
```bash
make unite
```

**Sandboxed / CI environments without system libraries:**
You cannot compile the project. Review code changes manually or push to CI (GitHub Actions) which installs all required packages. Do not attempt `make` if Lua dev headers or `libs/platform` are absent.

**Running tests (requires a full build):**
```bash
make test-headless          # layout tests, no display needed
xvfb-run make test          # full tests, needs virtual framebuffer
```

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
│   ├── MODULE_XML_GUIDE.md   # How to write module .cgen files
│   └── schemas/    # DTD schema files for XML validation
├── tools/           # Code-generation toolchain
│   └── codegen/     # C codegen host and generator plugins
├── mkdocs.yml       # MkDocs site configuration
└── samples/         # Example projects
```

---

## Specialist Areas

ORCA is a C game engine with a layered architecture.  Working effectively on any given part requires a different mindset.  The table below maps each area to the files it owns and the skills it demands — use it to orient yourself before diving in.

| Area | Files | Key skills |
|---|---|---|
| **Core / Object system** | `source/core/`, `include/orca.h`, `include/orcadef.h` | C, FNV1a hashing, message dispatch, `HANDLER` macro, component lifecycle |
| **Renderer** | `source/renderer/` | OpenGL ES, GLSL shaders, texture/mesh boxing (`BOX_PTR`), `ViewEntity` / `ViewDef` structs |
| **Geometry / Math** | `source/geometry/` | `vec2`, `vec3`, `mat4`, quaternion, `box3` math in C |
| **UIKit plugin** | `plugins/UIKit/` | Component architecture, layout engine (`MeasureOverride`/`ArrangeOverride`), Node2D message routing |
| **SceneKit / SpriteKit plugins** | `plugins/SceneKit/`, `plugins/SpriteKit/` | 3D scene graph, camera/light components, 2D sprite animation |
| **Lua scripting bridge** | `source/core/object/object_lua_*.c`, `share/orca/` | Lua C API, property binding, `send`/`fetch` dispatch, `behaviour.lua` |
| **Code generation toolchain** | `tools/codegen/`, `*.cgen` module files | C, libxml2, generator plugins, Makefile |
| **Build system** | `Makefile`, `orca.xcodeproj/` | GNU Make, pkg-config, Linux/macOS cross-platform C, Xcode project structure |
| **Test harness** | `tests/`, `source/orca.c` (`RunTest`) | Lua test patterns, `assert`, `xvfb-run`, headless layout testing |
| **Documentation** | `docs/`, `mkdocs.yml` | Markdown, MkDocs Material, `.cgen` API documentation |
| **DarkReign / asset formats** | `plugins/DarkReign/` | Binary format parsing, palette-indexed sprites, `.pz2` package files |

## Makefile Style

Keep Makefiles concise and shaped like the repository. Prefer `find`/pattern rules over hand-maintained file lists, build from the repository root when paths describe repository files, and keep command lines readable enough to copy directly. For code generation, the preferred form is:

```bash
build/bin/cgen source/core/core.cgen build/plugins/codegen/libexport.so > generated/core/core_export.c
```

Avoid verbose flag syntax or directory hopping when a short root-relative command is clear.

## Custom Agent Personas

Use these personas when delegating work or framing implementation choices.

### 1) App Developer Agent (MoonScript + Lapis-Inspired + Tailwind + Local Dialect)

- Focus: Orca application development using MoonScript with Lapis-inspired structure and conventions.
- Sources of truth:
  - `docs/lapis/lapis-user-guide.md`
  - `docs/lapis/lapis-internals.md`
  - `docs/skills/Orca-App-MoonScript-Lapis-Builder.skill.md`
- Behavior:
  - Uses Tailwind utilities and the local Orca UI dialect as much as possible.
  - Applies Lapis user-guide patterns to route/view/widget/layout composition in Orca.
  - Proactively provides feedback when an Orca primitive is missing and proposes a Lapis-inspired improvement.
  - For modal dialogs, prefer `<Popup>` templates that fill the screen via their background/overlay and are launched with the inline XML shorthand `{Screen.ShowModal Path=...}`; avoid setting root `Width`/`Height` on popups unless the popup truly needs custom bounds.

### 2) Lua Engine Agent (Lua-First, MoonScript-Interop, Lapis Internals)

- Focus: Lua-side Orca engine/application framework systems and APIs.
- Sources of truth:
  - `docs/lapis/lapis-internals.md`
  - `docs/lapis/lapis-user-guide.md`
  - `docs/skills/Orca-Lua-Lapis-Internals-Architect.skill.md`
- Behavior:
  - Prioritizes clean Lua APIs and runtime semantics.
  - Accounts for MoonScript subclassing/inheritance usage patterns (for example `Application`).
  - Builds systems heavily inspired by Lapis internals where they improve Orca architecture.

### 3) C Engine Agent (C-Runnable Core + XML/WPF/WinAPI-Inspired Architecture)

- Focus: C engine architecture and runtime behavior, preserving C-only operability and tests.
- Sources of truth:
  - `source/core/core.cgen`
  - `docs/way-of-working.md`
  - `docs/skills/Orca-C-Engine-WPF-WinAPI-Architect.skill.md`
- Behavior:
  - Keeps C engine runnable without requiring Lua/MoonScript.
  - Coordinates with Lua API developer to negotiate and stabilize APIs.
  - Uses message-driven architecture inspired by WPF/WinAPI (WinProc-like dispatch style) and strong systems design practice.

### Navigating by intent

| "I want to…" | Start here |
|---|---|
| Add a new component | [Way of Working → Component workflow](docs/way-of-working.md) |
| Understand message routing | [Object + Component System](docs/architecture/object-component-system.md) → Message Dispatch |
| Build a modal popup | `samples/Example/Screens/GetStartedPopup.xml` and `{Screen.ShowModal Path=...}` loading a `<Popup>` template; keep the popup root unbounded unless you need custom sizing |
| Add a UI widget to UIKit | `plugins/UIKit/UIKit.cgen` + `plugins/UIKit/Button.c` as a reference |
| Add a property binding or formula | `source/core/property/` + Module Codegen Guide |
| Change what Lua exposes | Edit the module `.cgen`, run `make modules` |
| Debug a rendering issue | `source/renderer/r_main.c`, `r_draw.c`; check `BOX_IS_PTR` before dereferencing mesh/shader fields |
| Add or fix a test | `tests/` — look at `test_styles_lua.lua` or `test_state_manager.lua` for patterns |
| Build the project locally | See [Building the Project](#building-the-project) above; install required system dependencies first |

---

## Module Structure

Every C module lives in `source/<module>/` or `plugins/<name>/` and follows the same structure:

```
<module>/
├── <module>.xml            # API definition — source of truth for public interface
└── *.c / *.h               # Hand-written implementation files

generated/<module>/
├── <module>.h              # Generated C header (ignored by git)
├── <module>_properties.h   # Generated property hash constants (ignored by git)
└── <module>_export.c       # Generated Lua binding code included by the module main file
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

## Object + Component System

ORCA uses an **Object/Component** architecture. An `Object` is the universal scene node — a named, hierarchical container. Functionality is added by attaching **components** (also called classes) to it.

Lua can call message handlers directly with a capitalized method name on any object that owns the handler, for example `self:MessageName(...)` or `screen:SetModalObject(modal)`. This is the preferred shorthand for dispatching component messages from Lua when the method name matches the message name.

### Object (`source/core/object/object_internal.h`)

Every scene element is an `Object`. Objects form a parent→children tree. Key fields:

- Identity: `Name`, `ClassName`, `identifier` (FNV1a hash), `SourceFile`
- Hierarchy: `parent`, `children` (first child), `next` (next sibling)
- Heap-allocated storage for dynamic/extra property values
- A `lua_State *domain` for scripting

```c
struct component* components;   // attached component chain
struct Property* properties; // property table (name, type, value storage, etc.)
```

Fields marked as planned replacements are being progressively replaced by proper components (see "Object Struct Refactoring" below).

### Components (`source/core/component.c`)

A component is a block of typed data + a message-handler function attached to an Object. Each component type is described by a `ClassDesc` (`include/orca.h`):

```c
struct ClassDesc {
  objectProc_t      ObjProc;          // message handler function
  struct PropertyType const *Properties;       // property descriptor table
  lpcString_t       ClassName;        // human-readable name (e.g. "Button")
  lpcString_t       DefaultName;      // name used when none supplied
  lpcString_t       ContentType;      // for package auto-detection
  lpcString_t       Xmlns;            // XML namespace
  uint32_t          ParentClasses[16];// base class IDs, zero-terminated
  uint32_t          NumProperties;    // property count
  uint32_t          ClassID;          // FNV1a hash of ClassName
  uint32_t          ClassSize;        // sizeof(struct ClassName)
  uint32_t          MemorySize;       // total size including parents
  void const       *Defaults;         // default values struct
  bool_t            ClassFlags;     // cannot be instantiated standalone
};
```

## Popup Convention

- Modal popups should be modeled as `Popup` objects, not `Screen` placeholders.
- `{Screen.ShowModal Path=...}` points to the popup template asset to instantiate, not a scene-tree path.
- Popups generally rely on a full-screen background/overlay and should not set root `Width`/`Height` unless there is a specific layout need.
- Use `Popup.ClosePopup` to dismiss the popup and return its result.

## XML Message Dispatch Shorthand

- Inline XML can dispatch any registered message type with the WPF-style shorthand, for example `{Node.RightButtonUp}`, `{Popup.ClosePopup}`, or `{Screen.ShowModal Path=...}`.
- The shorthand lowers directly to the generated dotted message action class.
- Named arguments map to the registered message payload fields, including short field names like `Path=...` when the message defines `Screen_ShowModalEventArgs.Path`.
- Use commas between named arguments when helpful, and nested `{...}` expressions are allowed inside payload values.
- Prefer this shorthand for concise dispatch; use an explicit dotted action element when nested XML reads better.

**`objectProc_t` signature** — note the `void* cmp` second parameter:

```c
typedef LRESULT (*objectProc_t)(struct Object *obj,   // owning Object
                                void*       cmp,   // component data block
                                uint32_t    msg,   // message ID (passed through; generated Procs mask with MSG_DATA_MASK)
                                wParam_t    wParam,
                                lParam_t    lParam);
```

`cmp` is the component's `pUserData` (already allocated alongside the `struct component` header). Handlers cast it implicitly to `struct ClassName*` in C.

**Standalone vs. component:**

| XML attribute | Macro in `*_export.c` | Intended usage |
|---|---|---|
| *(none)* | `REGISTER_CLASS` | May be created standalone or attached as a component |
| `` | `REGISTER_CLASS` | Intended to be attached to an existing object via `addComponent` |

`` should be treated as an architectural convention. `OBJ_AddComponentByName` (the Lua bridge) enforces it with a `luaL_error`; direct C callers of `OBJ_AddComponent(pobj, class_id)` are unrestricted. Call `OBJ_RegisterClass(&_ClassName)` at module init. Call `OBJ_AddComponent(obj, ID_ClassName)` (C) or `obj:addComponent("ClassName")` (Lua) to attach at runtime.

### HANDLER macro — the message-handler pattern

Every message a component handles is declared with `<handle message="NS.Event"/>` in XML. This generates a forward-declaration in `*_export.c` using the `HANDLER` macro and a switch case in the component's Proc. The implementation goes in the companion `.c` file:

```c
// *_export.c (generated) — forward-declarations:
HANDLER(AnimationPlayer, Object, Start);
HANDLER(AnimationPlayer, Object, Animate);

// AnimationPlayer.c (hand-written) — implementations:
HANDLER(AnimationPlayer, Object, Start) {
    // hObject         — the owning Object
    // pAnimationPlayer — this component's data (= cmp, already cast)
    // pStart          — Object_StartMsgPtr (empty for this message)
    pAnimationPlayer->CurrentTime = 0.0f;
    return FALSE;
}

HANDLER(AnimationPlayer, Object, Animate) {
    if (!pAnimationPlayer->Playing) return FALSE;
    // advance CurrentTime, evaluate curves…
    return FALSE;
}
```

`HANDLER(CLASS, NS, EVENT)` expands to:
```c
LRESULT CLASS##_##EVENT(struct Object* hObject,
                        struct CLASS*  p##CLASS,
                        wParam_t       wParam,
                        NS##_##EVENT##MsgPtr p##EVENT)
```

### Create-time defaults

`HANDLER(ClassName, Object, Create)` runs immediately after component allocation, before property values are applied. For initialization defaults in `Create`, assign the values directly; do not gate defaults with "already set" checks.

### Add a new component type — the mandatory workflow

**Rule: `.cgen` → codegen → handlers → Xcode → tests. Do not skip or reorder any step.**

Skipping the `.cgen` step means `struct MyComponent`, the message IDs, and the `REGISTER_*` macro are never generated — your C file will not compile.  Skipping tests means silent integration failures that are invisible until runtime.

#### Step 1 — Declare in the module `.cgen`

Add a `<class>` to the relevant `.cgen` file (e.g. `source/core/core.cgen`). Every handler must have a matching `<handle>` entry, and every message the component dispatches must have a `<message>` declaration. **Handlers without `<handle>` entries are orphaned — they will never be called.**

```xml
<class name="MyComponent" >
  <summary>What this component does.</summary>
  <handles>
    <handle message="Object.Start"/>
    <handle message="Object.Animate"/>
  </handles>
  <properties>
    <property name="Speed" type="float" default="1.0">Movement speed</property>
  </properties>
  <messages>
    <message name="SpeedChanged" routing="Direct">
      <fields>
        <field name="NewSpeed" type="float"/>
      </fields>
    </message>
  </messages>
</class>
```

Use `` for components meant to augment existing objects.  Omit it for standalone components that can be created as root objects (like `AnimationClip`).

#### Step 2 — Run code generation

```bash
make modules
```

This regenerates `<module>.h` (struct + accessor), `<module>_properties.h` (FNV hash IDs), and `<module>_export.c` (Proc switch + REGISTER macro + Lua bindings).  **Verify the generated `MyComponentProc` switch now contains `case` entries for every declared message.**  An empty `switch {}` means the `.cgen` step was incomplete.

#### Step 3 — Implement the C handlers

Create `source/<module>/components/MyComponent.c`:

```c
#include <source/core/core_local.h>
#include "<module>_properties.h"

HANDLER(MyComponent, Object, Start) {
    pMyComponent->Speed = 1.0f;
    return FALSE;
}

HANDLER(MyComponent, Object, Animate) {
    if (!pMyComponent->Speed) return FALSE;
    // per-frame logic
    return FALSE;
}
```

The `HANDLER` macro expands to the correct function signature; `*_export.c` forward-declares each handler and wires it into the generated `MyComponentProc` switch.

**Do not `#include <UIKit/UIKit.h>` (or any plugin header) from `source/core/`.** Core must not depend on plugins; this is an architectural violation that causes circular build dependencies.

Plugins must not link against other plugins or call another plugin's exported helper symbols. When one plugin needs behavior owned by another plugin, expose that behavior as an object/component message in the owning plugin's `.cgen` file and dispatch it through `OBJ_SendMessageW` / `_SendMessage`. Header-level knowledge of shared component/message types is acceptable when codegen requires it, but runtime coupling between plugins should flow through the message system.

#### Step 4 — Register the class at module init

The `REGISTER_CLASS` / `REGISTER_CLASS` macro in `*_export.c` defines `_MyComponent`.  The generated `on_mymodule_registered` callback must call:

```c
OBJ_RegisterClass(&_MyComponent);
```

If the module's `on-luaopen` callback is hand-written (rather than auto-generated), add the call there.

#### Step 5 — Register the new file in the Xcode project

New `.c` files under `source/core/components/` or `plugins/*/` must be added to `orca.xcodeproj/project.pbxproj` in **four** places:

1. `PBXBuildFile` section — one line: `<UUID> /* MyComponent.c in Sources */`
2. `PBXFileReference` section — one line: `<UUID> /* MyComponent.c */`
3. `PBXGroup` section (under the `components` group) — reference UUID
4. `PBXSourcesBuildPhase` section — build file UUID

Omitting this step means the file is silently excluded from the macOS/Xcode build even though the Makefile build succeeds.

#### Step 6 — Write tests

Add at minimum a Lua test file (e.g. `tests/test_mycomponent.lua`) that:
- Creates an object, attaches the component
- Exercises each property and message handler
- Asserts observable side effects

Without tests, an empty `switch {}` in a generated Proc (caused by missing `<handle>` entries in XML) will silently do nothing and remain undetected until production.  All successful component refactors in this repo (StyleController, StateManager, AnimationPlayer) had tests written alongside.

#### Step 7 — Build and test

```bash
make unite               # C-only build (fast)
xvfb-run make test       # full test suite
```

---

## Object Struct Refactoring

The `struct Object` currently has a number of **legacy embedded fields** — timers, state managers, style sheets, class lists — that predate the component architecture and create tight coupling in `OBJ_Release`.

**Architectural goal: zero subsystem fields in Object.** Every subsystem moves into a proper `component` component:

| Legacy field | Target component | Status |
|---|---|---|
| `animation` / `animlib` | `AnimationPlayer` / `AnimationClip` | **Done** |
| `animations` (tweens) | `PropertyAnimation` | **Done** |
| `timers` | `Timer` | Planned |
| `stateManager` | `StateManager` | Planned |
| `stylesheet` / `classes` | `StyleSheet` / `StyleClass` | Planned |
| `aliases` | `Alias` | Planned |

**Migration checklist for each field:**
1. Implement the component with `<handles>` for all messages it needs.
2. Register it as `component` via `REGISTER_CLASS`.
3. Remove the field from `struct Object`.
4. Remove the `_GetXxx(obj)` macro from `core_local.h`.
5. Remove the manual release call from `OBJ_Release`.
6. Remove the corresponding `kComp*` entry from `enum component_type`.

When adding new functionality, **always use a component** — never add new fields to `struct Object`.

---

## C Macros Quick Reference

Full reference: `docs/architecture/macros-reference.md`. The most important macros for day-to-day component work:

### `HANDLER(CLASS, NS, EVENT)` — implement a message handler

```c
// In MyComponent.c:
HANDLER(MyComponent, Object, Start) {
    // hObject         — owning Object
    // pMyComponent    — component data (struct MyComponent*)
    // pStart          — Object_StartMsgPtr
    pMyComponent->Value = 0.0f;
    return FALSE; // continue dispatching to other components
}
```

Declared (forward) in `*_export.c` from `<handle message="NS.Event"/>` XML. Returns non-zero to stop dispatch.

### `DECL` / `ARRAY_DECL` — property table entry (generated, do not write by hand)

```c
DECL(SHORT_HASH, ClassName, PropertyName, StructField, kDataTypeFloat)
ARRAY_DECL(SHORT_HASH, ClassName, Keyframes, Keyframes, kDataTypeStruct, .TypeString = "Keyframe")
```

Builds a `PropertyType` row: name, category, identifiers, offset, size, type.

### `_SendMessage(OBJECT, CLASS, MESSAGE, …)` — dispatch a typed message

```c
_SendMessage(object, Object, Start);
_SendMessage(object, Object, PropertyChanged, .Property = myProp);
_SendMessage(object, AnimationPlayer, Play);
```

### `REGISTER_CLASS(NAME, parent_ids…)` / `REGISTER_CLASS(NAME, …)` — define ClassDesc (generated)

```c
REGISTER_CLASS(AnimationClip, 0);              // standalone
REGISTER_CLASS(AnimationPlayer, 0); // component
```

### `FOR_EACH_OBJECT(var, parent)` / `FOR_EACH_LIST(TYPE, var, head)`

```c
FOR_EACH_OBJECT(child, object) { OBJ_Animate(L, child); }
FOR_EACH_LIST(struct timer, t, _GetTimers(obj)) { /* … */ }
```

### Message ID routing bits

Switch cases **must** mask with `MSG_DATA_MASK`:
```c
case ID_Object_Start & MSG_DATA_MASK:   // ← correct
case ID_Object_Start:                   // ← wrong — routing bits included
```

### `GetClassName(obj)` accessor macro

Generated in `*_properties.h` for each class. Returns `NULL` if the component is not attached:
```c
struct AnimationPlayer *ap = GetAnimationPlayer(object);
if (ap) { ap->Playing = TRUE; }
```

---

ORCA supports two distinct kinds of plugins:

### C Component Plugins (Object Plugins)

These are shared libraries (`.so` / `.dll`) that register new `ClassDesc` component types into the engine. **UIKit is the primary example** — it adds 16+ UI component types (Button, Label, ImageView, Grid, …).

**Plugin structure:**

```
plugins/UIKit/
├── UIKit.cgen          # API definition → generates bindings
├── UIKit.h            # Generated header
├── UIKit_export.c     # Generated Lua bindings + luaopen_orca_UIKit()
├── UIKit_properties.h # Generated property hashes
├── UIKit.c            # on_ui_module_registered() callback
├── Button.c           # One .c file per component
├── Label.c
└── …
```

**Plugin loading flow:**
1. The engine discovers `.so` files in the plugins directory at startup.
2. It calls each plugin's `luaopen_orca_<Name>(L)` function (auto-generated in `*_export.c`).
3. The `on_luaopen` callback (e.g. `on_ui_module_registered`) calls `OBJ_RegisterClass()` for each component type.
4. Lua scripts can then instantiate components: `local btn = UIKit.Button()`.

**Message handling** — component message handlers use the `HANDLER` macro. The generated Proc dispatches to them:

```c
// In Button.c — implemented with HANDLER:
HANDLER(Button, Node, LeftButtonUp) {
    // hObject  — owning Object
    // pButton  — component data (struct Button*)
    // pLeftButtonUp — Node_LeftButtonUpMsgPtr
    SV_PostMessage(hObject, "Click", 0, hObject);
    return TRUE; // handled — stop dispatch
}

// The generated ButtonProc in UIKit_export.c calls:
//   case ID_Node_LeftButtonUp & MSG_DATA_MASK:
//       return Button_LeftButtonUp(object, cmp, wparm, lparm);
```

The `objectProc_t` signature (for reference — use `HANDLER`, not a raw Proc):
```c
LRESULT MyProc(struct Object *obj, void* cmp, uint32_t msg, wParam_t wParam, lParam_t lParam);
//                              ^^^^^^^^ component data block — cast to struct ClassName*
```

**Inheritance** — set `ParentClasses` to inherit properties and message handling:

```c
static ClassDesc ButtonClass = {
  .ClassName    = "Button",
  .ParentClasses = { ID_Node2D, 0 },  // inherits Node2D layout + rendering
  …
};
```

### Lua Script Plugins

Pure Lua modules loaded via `require "orca.<module>"`. They can:

- Call engine APIs exposed by C plugins
- Register new Lua classes by calling engine functions
- Implement game/app logic, data binding, routing, etc.

Script plugins live in the project directory and are loaded by the project's entry-point Lua file.

---

## Module Codegen System

Every C module exposes its API via a `<module>.cgen` file co-located with its C sources. The C codegen host and plugins generate:

| Generated file | Generator plugin |
|---|---|
| `<module>.h` | `build/plugins/codegen/libheader.so` |
| `<module>_properties.h` | `build/plugins/codegen/libproperties.so` |
| `<module>_export.c` | `build/plugins/codegen/libexport.so` |

Run `make modules` from the repository root to regenerate all files. `tools/Makefile` discovers module files with `find source plugins -name '*.cgen'`.

### Key XML elements

See `docs/MODULE_XML_GUIDE.md` for full reference. Quick summary:

| Element | Purpose |
|---|---|
| `<module>` | Root element |
| `<enums>` | C enum definition |
| `<struct>` | C struct — contains `<fields>` and `<methods>` containers |
| `<interface>` | Abstract interface — contains `<methods>` and `<messages>` containers |
| `<class>` | Component — contains `<handles>`, `<properties>`, `<fields>`, `<methods>`, `<messages>` containers |
| `<function>` | Global function |
| `<message>` | Message declaration — contains `<fields>` container |
| `<method>` | Method on a struct/interface/class (inside `<methods>`) |
| `<topic>` | Section separator **inside `<methods>`** in an `<interface>` — has `title` attr (docs only) |
| `<field>` | Data member (inside `<fields>`) |
| `<property>` | Component property (inside `<properties>`, exposed to Lua) |
| `<handle>` | Message handler declaration (inside `<handles>`) |
| `<arg>` | Function/method argument |
| `<returns>` | Return value description |
| `<includes>` | Container for `<include>` directives at module level |
| `<externals>` | Container for `<external>` type references at module level |

### `<topic>` — inline section separator inside `<methods>`

`<topic>` is a **sibling separator** placed between `<method>` elements **inside a `<methods>` container** within an `<interface>`. It marks the start of a named section in generated documentation.

```xml
<interface name="Object" prefix="OBJ_" export="Object" no-check="true">
  <summary>Core engine host object.</summary>
  <methods>
    <topic title="Lifecycle">Manages object creation, initialization, and destruction.</topic>
    <method name="Awake" lua="true">
      <summary>Initializes the object when loaded.</summary>
    </method>

    <topic title="Hierarchy">Navigates and manipulates the parent-child relationship tree.</topic>
    <method name="AddChild">
      <summary>Add a child object.</summary>
      <arg name="child" type="Object" pointer="true">Child to add</arg>
    </method>
  </methods>
  <messages>
    <message name="Create" routing="Direct"/>
  </messages>
</interface>
```

- `title` attribute (required) — the section heading in generated docs
- Text content (optional) — description paragraph rendered below the heading
- Methods **after** a `<topic>` belong to that section until the next `<topic>` or end of `<methods>`
- `<topic>` is invisible to code generation: `getMethods()` (XPath `./methods/method[@name]`) traverses all methods in the container regardless of intervening topic separators

Use topics when an interface has more than ~10 methods. See `source/core/core.cgen` for a full example with 10 topic separators covering 73 methods.

The DTD allows `(method | topic)*` as the `<methods>` content model. `struct` and `class` elements do **not** support topics.

### `<class>` grouped structure

A `<class>` element uses grouped container children:

```xml
<class name="SKNode" parent="Node">
  <summary>...</summary>
  <details>...</details>
  <xmlns>http://schemas.corepunch.com/orca/2006/xml/presentation</xmlns>
  <handles>
    <handle message="Node.UpdateMatrix"/>
  </handles>
  <properties>
    <property name="Position" type="vec2">Position</property>
  </properties>
  <fields>
    <field name="_matrix" type="mat4"/>
  </fields>
  <methods>
    <method name="DoSomething" lua="true">...</method>
  </methods>
  <messages>
    <message name="Render">
      <fields>
        <field name="ViewDef" type="ViewDef" pointer="true"/>
      </fields>
    </message>
  </messages>
</class>
```

---

## C Code-Generation Tool

The code generation toolchain converts `.cgen` module definitions into `.h`, `_properties.h`, and `_export.c` files. The root `Makefile` builds `build/bin/cgen` and all `tools/codegen/plugins/*.c` files into `build/plugins/codegen/lib*.so`.

### Running code generation

```bash
# Regenerate all modules (headers + exports + properties)
make modules
```

### Running a single generator manually

```bash
make codegen-host
build/bin/cgen source/core/core.cgen build/plugins/codegen/libheader.so > generated/core/core.h
build/bin/cgen source/core/core.cgen build/plugins/codegen/libproperties.so > generated/core/core_properties.h
build/bin/cgen source/core/core.cgen build/plugins/codegen/libexport.so > generated/core/core_export.c
```

---

## Key Files for Module Codegen Work

| File | Role |
|---|---|
| `docs/schemas/module.dtd` | XML schema for module files — update when adding new elements |
| `tools/codegen/src/model.c` | C parser/model loader — add support for new elements |
| `tools/codegen/plugins/header.c` | C header generator |
| `tools/codegen/plugins/properties.c` | Property/hash header generator |
| `tools/codegen/plugins/export.c` | Lua export generator |
| `source/core/core.cgen` | Reference example for complex interfaces with topics |

---

## Feature Development Workflow

When implementing a new component, property, message, or any other API addition, always follow this order:

1. **Codegen first** — Define the public API in the module's `.cgen` file (class, properties, messages, handles, methods).
2. **Code generation** — Run `make modules` to regenerate `*.h`, `*_properties.h`, and `*_export.c` from the updated `.cgen` files.
3. **HANDLER implementations** — Write the hand-coded `.c` handler bodies using the `HANDLER` macro; never edit generated files.
4. **Tests** — Add or update tests (Lua layout tests, C unit tests) and verify with `make test-headless` / `xvfb-run make test`.

> **Never skip step 2.** Editing generated files by hand will be overwritten on the next `make`. Commit the `.cgen`, handwritten source, and build-rule changes, not the ignored `generated/` outputs.

---

## Common Tasks

### Add a new element type to the codegen schema

1. Add the element to `docs/schemas/module.dtd`
2. Add parser/model support to `tools/codegen/src/model.c`
3. Update the relevant generator plugin under `tools/codegen/plugins/`
4. Update `docs/MODULE_XML_GUIDE.md` with documentation

### Add a new method to an interface in `.cgen`

```xml
<methods>
  <method name="MyMethod" export="myMethod" lua="true">
    <summary>One-line description.</summary>
    <details>Detailed explanation.</details>
    <arg name="value" type="int">The value to pass</arg>
    <returns type="bool">True on success</returns>
  </method>
</methods>
```

If the interface uses `<topic>` separators, place the method inside `<methods>` after the appropriate `<topic title="...">` separator.

### Run code generation

```bash
cd tools
make                    # Regenerate all headers and export files
make docs               # Regenerate API documentation Markdown
make dtd                # Regenerate orca.dtd (UI element schema)
```

---

## Documentation Website

The site is built with MkDocs Material and deployed to GitHub Pages on push to `main`.

- Config: `mkdocs.yml`
- Source: `docs/` directory
- New pages must be added to the `nav:` section in `mkdocs.yml`

To preview locally:

```bash
pip install mkdocs-material
mkdocs serve
```

---

## Key Findings & Gotchas

Discoveries made during development that are non-obvious or can trip up developers.

### Renderer: Palette-Indexed Textures

- `palette_t = uint32_t[256]` — each entry is `0x00RRGGBB`; **index 0 is always transparent** (alpha = 0); all other entries get alpha = 255.
- Set `ViewEntity.palette` to a `struct color32 const*` (256-entry array). Inside `R_DrawEntity`:
  1. If `ent.shader` is `NULL`, the renderer **automatically selects `SHADER_CINEMATIC`** (palette look-up shader).
  2. `R_SetPalette` is called to upload the LUT to the `TX_CINEMATICPALETTE` texture slot.
- `R_SetPalette` is **`static`** (private to `r_main.c`) — never call it directly; set `ViewEntity.palette` instead.
- `TX_CINEMATICPALETTE` is a **shared** 256×1 texture slot. The cinematic player and palette-indexed sprites both use it; whichever draws last wins.

### Renderer: Shader and Mesh Boxing

- Use `BOX_PTR(Shader, SHADER_*)` to select a built-in shader type without looking up the real pointer.
- Use `BOX_PTR(Mesh, MD_*)` for built-in geometry types (rectangle, capsule, ninepatch, …).
- **Always** check `BOX_IS_PTR((uintptr_t)ptr)` before dereferencing a `mesh` or `shader` field — boxed values are small integers cast to pointers and must never be dereferenced.
- Shader selection in `R_DrawEntity` (in priority order): palette → SHADER_CINEMATIC; no shader → SHADER_UI (or SHADER_2D_RECT on macOS IOSurface); real pointer → use directly; boxed tag → `tr.shaders[tag]`.

### ClassDesc — Extended Fields

Beyond the fields shown in the architecture docs, `ClassDesc` also has:

```c
lpcString_t DefaultName;   // name given to objects of this class when none is provided
lpcString_t ContentType;   // used for package auto-detection and editor filtering
lpcString_t Xmlns;         // XML namespace for package auto-detection
uint32_t    MemorySize;    // total instance size including all components
```

Use `OBJ_EnumClasses(superclassID, callback, param)` to iterate all registered classes that inherit from a given class ID.

### Lua Module Loading

- All built-in modules are registered in `package.preload` by `luaopen_orca` **before** any project code runs — they execute lazily on first `require`.
- The `orca` table has a `__index` metamethod that auto-requires `orca.<key>` on first access and caches the result, so `orca.geometry` and `require "orca.geometry"` are equivalent.
- On WebGL, `package.cpath` is not extended (no `.so` loading). Plugin modules are compiled into the WASM binary and pre-registered via an auto-generated `plugins_luaopen.h`.
- **Gotcha — init order:** `luaopen_*` runs on the first `require`, not at registration. If your module init has side effects depending on another module, call `require` explicitly — don't assume registration order.

### Module Codegen — Grouped Container Structure

All child elements in `<class>`, `<struct>`, `<interface>`, and `<message>` are grouped into container elements:
- `<class>`: `<handles>` (with `<handle message="..."/>` children), `<properties>`, `<fields>`, `<methods>`, `<messages>`
- `<struct>`: `<fields>`, `<methods>`
- `<interface>`: `<methods>` (may also contain `<topic>` separators), `<messages>`
- `<message>`: `<fields>`
- `<module>` top-level: `<includes>`, `<externals>`

The C parser (`tools/codegen/src/model.c`) preserves direct parent/child relationships in a flat node array. Generator plugins should use `cg_foreach(model, parent_id, kind, node)` for direct children rather than scanning descendants unless the behavior explicitly needs dependency lookup.

### Dark Reign SPR Format — Key Gotchas

- Two sprite types: `"RSPR"` (normal) and `"SSPR"` (shadow). Shadow sprites fill all opaque pixels with **palette index 47** instead of reading pixel data from the stream.
- Rotation 0 on disk means the sprite faces **right**. The loader shifts by `nrots/4` to align sprites with the conventional "up" direction.
- `off_bits` is derived, not stored: `32 + 4*nanims*nrots + 16*nsects + 4*nanims + 8*npics + 4`.
- The `TX_CINEMATICPALETTE` slot doubles as the sprite palette for palette-indexed textures.

### WebGL Build

- Use `make webgl` to build; requires `emcc` in PATH.
- Run `emmake make wasm-deps` once to compile lua5.4, libxml2, and liblz4 for WASM.
- The engine uses `-sASYNCIFY=1`: the Lua main loop calls `emscripten_sleep(0)` when the event queue is empty, which yields to the browser and resumes on the next tick.
- `orca.network` and `orca.editor` are excluded from WebGL builds.

### Renderer Lifecycle — `tr.buffer` as Initialization Sentinel

`renderer_Init()` (`source/renderer/r_main.c`) is the only function that creates the OpenGL context and calls `R_InitBuffers()` → `glGenBuffers()`, which sets `tr.buffer` to a non-zero value. Requiring `orca.renderer` (via `luaopen_orca_renderer`) only calls `axInit()` — it does **not** initialize the full renderer.

**Gotcha:** Calling `renderer_Shutdown()` without having first called `renderer_Init()` crashes in `axMakeCurrentContext()` because no GL context exists. The guard `if (!tr.buffer) return;` at the top of `renderer_Shutdown` short-circuits this path. Always check `tr.buffer` before performing any GL teardown.

This matters in test harnesses (`RunTest` in `source/orca.c`) that `require "orca.renderer"` for XML parsing but never create a window.

### XML Attribute Iterator — `xmlGetProp` vs `xmlNodeListGetString`

The XML attribute iterator in `source/parsers/p_xml.c` previously used `xmlNodeListGetString(attr->parent->doc, attr->children, 1)` to read attribute values. When `XML_PARSE_COMPACT` is active (the default flag in `include/orcadef.h`), libxml2 may store small text nodes differently, causing `xmlNodeListGetString` to return `NULL` even for non-empty attributes. This manifested as `setName(nil)` errors in `file-xml.lua`.

**Fix:** Use `xmlGetProp(node, attr->name)` (or `xmlGetNsProp` for namespaced attributes) — the canonical libxml2 API for reading attribute values, which is immune to compact-mode storage differences. Always free the returned `xmlChar*` with `xmlFree`.

### Screen ResizeMode in Tests and Headless Contexts

`Screen.MeasureOverride` calls `axGetSize()` and overrides the screen's requested `Width`/`Height` with the actual window dimensions when `ResizeMode == kResizeModeCanResize` (the **default** value). On a virtual framebuffer (e.g., `xvfb-run` in CI), the window is 640×480, so any test that creates a `Screen { Width = 1000, Height = 1000 }` without setting `ResizeMode` will silently get a 640×480 screen — causing layout assertions to fail.

**Fix:** Always set `ResizeMode = "NoResize"` (Lua) or `ResizeMode="NoResize"` (XML attribute) for test screens that must keep their explicit dimensions.

```lua
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }
```

### Tailwind Colors Come From ThemeLibrary

- `tailwind.lua` defines class-based style utilities (`bg-*`, `text-*`, etc.), but color values come from `ThemeLibrary` / `orca.theme.colors`.
- Do not add or restore `config/tailwind.lua`; the Tailwind plugin does not load per-project Tailwind config.
- Missing `$...` keys (`FS_GetThemeValue: missing theme key ...`) are fixed by adding static `ThemeLibrary` entries in `package.lua`.
- Keep `samples/*/package.lua` declarative data only. Do not call `require` or runtime APIs from `package.lua`; project loading executes package chunks in a restricted project table environment.
- If a sample uses Tailwind and also relies on UIKit controls, define any project-specific colors in `ThemeLibrary` using the same keys used by classes and CSS variables (`accent`, `background`, `foreground`, `control-background`, etc.).

### Property VM — `Token_Release` Must Recurse into `args[]`

`Token_Release` in `source/core/property/property_runtime.c` walks `token->next` to release sibling tokens, but **does not** recurse into `token->args[]`. Every function-call token (`ADD`, `MUL`, `IF`, …) keeps its argument sub-tokens in `args[0..TOKEN_MAX_ARGS-1]`. Failing to release them leaks all arguments of every operator token.

**Fix:** After releasing `token->next`, loop over `token->args[i]` for `i < TOKEN_MAX_ARGS` and call `Token_Release` on each non-NULL arg before freeing the token itself.

### Property VM — `IF` Operator

The property VM supports an `IF(cond, true-val, false-val)` operator following Excel/formula semantics:

- Condition is **numeric** (`cond.value[0] != 0`) or **string** (non-empty string = truthy).
- The result type follows whichever branch is selected — works with int, float, and string registers.
- Registered in `op_call` alongside `ADD`, `MUL`, etc. via `CALL(IF)`.

```xml
<property name="Label">
  <Binding>IF(EQUAL({./IsActive}, 1), "On", "Off")</Binding>
</property>
```

### Component Development — What Goes Wrong

These anti-patterns have caused real integration failures.  Avoid them.

**Anti-pattern 1: Writing C handlers without declaring the class in `.cgen`**

If you create `source/core/components/Foo.c` with `HANDLER(Foo, ...)` macros but never add `<class name="Foo">` to the module `.cgen`, then:
- `struct Foo` is never generated → the file does not compile
- `FooProc` is never generated → the handlers have no dispatch table
- `REGISTER_CLASS(Foo)` is never generated → the class is never registered
- Message IDs like `ID_Foo_Bar` are never generated → any switch case silently resolves to 0

The fix is always: **write the `.cgen` first, run codegen, then write the C**.

**Anti-pattern 2: Declaring the class in XML but not wiring any `<handle>` entries**

A class with no `<handles>` generates an empty `FooProc` switch.  The component exists and can be attached to objects, but **it does nothing** — silently.  Every message that needs a response must have a matching `<handle message="..."/>` in XML and a `HANDLER(Foo, NS, Event)` implementation in C.

**Anti-pattern 3: Declaring and partly implementing a class but not connecting old and new code**

When migrating a global/singleton system into a component:
- The new component struct may exist in XML (generating the class)
- But the old implementation (e.g. a `static` global list) is never replaced
- Result: the component is registered and attachable, but its Proc does nothing; the old global code still runs outside the component lifecycle

Always verify the migration is *complete*: the old code path is deleted, and all behaviour is driven through the new component's message handlers.

**Anti-pattern 4: Including plugin headers from core**

`source/core/` must never `#include <UIKit/UIKit.h>` or any other plugin header.  The dependency direction is: **plugins depend on core; core does not depend on plugins**.  Violating this creates circular build dependencies, breaks cross-platform builds, and makes the core untestable in isolation.  If core code needs a type from UIKit (e.g. `struct Node`), refactor so the lookup goes through a generic message or a registered accessor.

**Anti-pattern 5: Skipping tests**

All component refactors that went smoothly (StyleController, StateManagerController, AnimationPlayer) had dedicated test files written alongside the implementation.  Refactors that ran into problems (Aliases, Locale) had none.  Always add `tests/test_<componentname>.lua` covering attach, property set/get, and at least one message roundtrip.

### System-Level Message Handlers — `SV_RegisterMessageProc`

Plugins and modules can register a **system-level message handler** (separate from per-object `ObjProc`) to receive raw platform events before they reach the object tree.

**Signature:**
```c
LRESULT my_handler(lua_State *L, struct AXmessage *msg);
```
Return `TRUE` to consume the event; `FALSE` to let subsequent handlers run. Handlers are called in reverse registration order (LIFO).

**Registration — always from the `on-luaopen` callback:**
```c
void on_myplugin_registered(lua_State *L) {
    SV_RegisterMessageProc(my_handler);
}
```
For conditional registration (e.g. feature-flagged):
```c
lua_getglobal(L, "MY_FLAG");
if (lua_toboolean(L, -1)) {
    SV_RegisterMessageProc(my_handler);
    axPostMessageW(NULL, kEventReadCommands, 0, NULL);  /* seed event pump if needed */
}
lua_pop(L, 1);
```

**Three canonical examples in the codebase:**
- `CORE_ProcessMessage` (`source/core/core_main.c`) — keyboard shortcuts and window paint/resize
- `ui_handle_event` (`plugins/UIKit/UIKit_message.c`) — mouse, keyboard, coroutine events for UI nodes
- `filesystem_handle_event` (`source/editor/server.c`) — HTTP-style editor command server loop on `kEventReadCommands`

**The `kEventReadCommands` loop pattern** used by `filesystem_handle_event`: register under a `SERVER` global guard, post the first `kEventReadCommands` event to start the pump, then re-arm at the end of each invocation with another `axPostMessageW(msg->target, kEventReadCommands, 0, NULL)`.

Full documentation: `docs/architecture/plugin-system.md` → "System-Level Message Handlers".
