# ORCA — Copilot Instructions

This file gives GitHub Copilot (and any AI coding assistant) the essential context needed to work effectively in the ORCA repository without having to re-discover it from scratch.

---

## Repository Setup

After cloning, initialize submodules before building or running code generation:

```bash
git submodule update --init --recursive
```

This pulls two required submodules:

| Submodule | Path | Purpose |
|---|---|---|
| `platform` | `libs/platform` | Cross-platform abstraction layer (window, input, audio) |
| `pyphp` | `tools/pyphp` | Python-to-PHP bridge for running the code generation templates |

The `pyphp` module must also be installed as a Python package before running `make` in `tools/`:

```bash
pip install -e tools/pyphp
```

---

## Building the Project

**Full build** (requires submodules + pyphp + system libraries):
```bash
make
```
This runs code generation (`make -C tools`) **then** C compilation. Both pyphp and all system libraries (lua5.4-dev, libxml2-dev, etc.) must be present.

**C-only build** (skips code generation — use when pyphp is unavailable):

The generated files (`*_export.c`, `*.h`, `*_properties.h`) are committed to the repository. If you are only modifying `.c` or `.h` implementation files (not `.xml` API definitions), you do **not** need to regenerate them. The full build will work if you have the system libraries:
```bash
make unite   # compile + link; skips 'make -C tools'
```
> **Warning:** running `make` or `make -C tools` when pyphp is **not** installed will silently **empty** all generated files (the shell redirect `> file` truncates the file even when the command fails). Always verify pyphp is available before running code generation.

**Sandboxed / CI environments without system libraries:**
You cannot compile the project. Review code changes manually or push to CI (GitHub Actions) which installs all required packages. Do not attempt `make` if Lua dev headers or `libs/platform` are absent — it will corrupt generated files.

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
│   ├── MODULE_XML_GUIDE.md   # How to write module XML files
│   └── schemas/    # DTD schema files for XML validation
├── tools/           # Code-generation toolchain
│   ├── model/       # PHP data model (module.php — XML parser)
│   ├── templates/   # PHP code-gen templates (header, export, docs, …)
│   ├── Makefile     # `make all` → headers+exports; `make docs` → Markdown docs
│   └── pyphp/       # pyphp submodule (install with `pip install -e tools/pyphp`)
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
| **Code generation toolchain** | `tools/`, `*.xml` module files | PHP (via pyphp), XML parsing, C codegen templates, Makefile |
| **Build system** | `Makefile`, `orca.xcodeproj/` | GNU Make, pkg-config, Linux/macOS cross-platform C, Xcode project structure |
| **Test harness** | `tests/`, `source/orca.c` (`RunTest`) | Lua test patterns, `assert`, `xvfb-run`, headless layout testing |
| **Documentation** | `docs/`, `mkdocs.yml` | Markdown, MkDocs Material, module XML → API doc pipeline |
| **DarkReign / asset formats** | `plugins/DarkReign/` | Binary format parsing, palette-indexed sprites, `.pz2` package files |

### Navigating by intent

| "I want to…" | Start here |
|---|---|
| Add a new component | [Way of Working → Component workflow](docs/way-of-working.md) |
| Understand message routing | [Object + Component System](docs/architecture/object-component-system.md) → Message Dispatch |
| Add a UI widget to UIKit | `plugins/UIKit/UIKit.xml` + `plugins/UIKit/Button.c` as a reference |
| Add a property binding or formula | `source/core/property/` + Module XML Guide |
| Change what Lua exposes | Edit the module XML, run `cd tools && make` |
| Debug a rendering issue | `source/renderer/r_main.c`, `r_draw.c`; check `BOX_IS_PTR` before dereferencing mesh/shader fields |
| Add or fix a test | `tests/` — look at `test_styles_lua.lua` or `test_state_manager.lua` for patterns |
| Build the project locally | See [Building the Project](#building-the-project) above; install submodules + pyphp first |

---

## Module Structure

Every C module lives in `source/<module>/` or `plugins/<name>/` and follows the same structure:

```
<module>/
├── <module>.xml            # API definition — source of truth for public interface
├── <module>.h              # Generated C header (do not edit by hand)
├── <module>_properties.h   # Generated property hash constants (do not edit)
├── <module>_export.c       # Generated Lua binding code (do not edit)
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

## Object + Component System

ORCA uses an **Object/Component** architecture. An `Object` is the universal scene node — a named, hierarchical container. Functionality is added by attaching **components** (also called classes) to it.

### Object (`source/core/object/object_internal.h`)

Every scene element is an `Object`. Objects form a parent→children tree. Key fields:

- Identity: `Name`, `ClassName`, `identifier` (FNV1a hash), `SourceFile`
- Hierarchy: `parent`, `children` (first child), `next` (next sibling)
- A union of subsystem pointers (see refactoring section below)
- A flat `data[]` byte buffer for property value storage
- A `lua_State *domain` for scripting

The subsystem union provides **indexed O(1) access** via `comps[kCompCount]` and named struct fields simultaneously:

```c
union {
    struct {
        struct component*          components;   // attached component chain
        struct Property*           properties;
        struct state_manager*      stateManager;
        struct style_class*        classes;
        struct style_rule*        stylesheet;
        struct timer*              timers;
        struct alias*              aliases;
    };
    void* comps[kCompCount]; // indexed by enum component_type
};
```

Fields marked as planned replacements are being progressively replaced by proper components (see "Object Struct Refactoring" below).

### Components (`source/core/component.c`)

A component is a block of typed data + a message-handler function attached to an Object. Each component type is described by a `ClassDesc` (`include/orca.h`):

```c
struct ClassDesc {
  objectProc_t      ObjProc;          // message handler function
  lpcPropertyType_t Properties;       // property descriptor table
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
  bool_t            IsAttachOnly;     // cannot be instantiated standalone
};
```

**`objectProc_t` signature** — note the `void* cmp` second parameter:

```c
typedef LRESULT (*objectProc_t)(lpObject_t obj,   // owning Object
                                void*       cmp,   // component data block
                                uint32_t    msg,   // message ID (passed through; generated Procs mask with MSG_DATA_MASK)
                                wParam_t    wParam,
                                lParam_t    lParam);
```

`cmp` is the component's `pUserData` (already allocated alongside the `struct component` header). Handlers cast it implicitly to `struct ClassName*` in C.

**Standalone vs. attach-only:**

| XML attribute | Macro in `*_export.c` | Intended usage |
|---|---|---|
| *(none)* | `REGISTER_CLASS` | May be created standalone or attached as a component |
| `attach-only="true"` | `REGISTER_ATTACH_ONLY_CLASS` | Intended to be attached to an existing object via `addComponent` |

`attach-only="true"` should be treated as an architectural convention. `OBJ_AddComponentByName` (the Lua bridge) enforces it with a `luaL_error`; direct C callers of `OBJ_AddComponent(pobj, class_id)` are unrestricted. Call `OBJ_RegisterClass(&_ClassName)` at module init. Call `OBJ_AddComponent(obj, ID_ClassName)` (C) or `obj:addComponent("ClassName")` (Lua) to attach at runtime.

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

### Add a new component type — the mandatory workflow

**Rule: XML → codegen → handlers → Xcode → tests. Do not skip or reorder any step.**

Skipping the XML step means `struct MyComponent`, the message IDs, and the `REGISTER_*` macro are never generated — your C file will not compile.  Skipping tests means silent integration failures that are invisible until runtime.

#### Step 1 — Declare in the module XML

Add a `<class>` to the relevant `.xml` file (e.g. `source/core/core.xml`). Every handler must have a matching `<handle>` entry, and every message the component dispatches must have a `<message>` declaration. **Handlers without `<handle>` entries are orphaned — they will never be called.**

```xml
<class name="MyComponent" attach-only="true">
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

Use `attach-only="true"` for components meant to augment existing objects.  Omit it for standalone components that can be created as root objects (like `AnimationClip`).

#### Step 2 — Run code generation

```bash
cd tools && make
```

This regenerates `<module>.h` (struct + accessor), `<module>_properties.h` (FNV hash IDs), and `<module>_export.c` (Proc switch + REGISTER macro + Lua bindings).  **Verify the generated `MyComponentProc` switch now contains `case` entries for every declared message.**  An empty `switch {}` means the XML step was incomplete.

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

**Do not `#include <plugins/UIKit/UIKit.h>` (or any plugin header) from `source/core/`.** Core must not depend on plugins; this is an architectural violation that causes circular build dependencies.

#### Step 4 — Register the class at module init

The `REGISTER_CLASS` / `REGISTER_ATTACH_ONLY_CLASS` macro in `*_export.c` defines `_MyComponent`.  The generated `on_mymodule_registered` callback must call:

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

**Architectural goal: zero subsystem fields in Object.** Every subsystem moves into a proper `attach-only` component:

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
2. Register it as `attach-only` via `REGISTER_ATTACH_ONLY_CLASS`.
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

### `REGISTER_CLASS(NAME, parent_ids…)` / `REGISTER_ATTACH_ONLY_CLASS(NAME, …)` — define ClassDesc (generated)

```c
REGISTER_CLASS(AnimationClip, 0);              // standalone
REGISTER_ATTACH_ONLY_CLASS(AnimationPlayer, 0); // attach-only
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
├── UIKit.xml          # API definition → generates bindings
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
HANDLER(Button, Node, LeftMouseUp) {
    // hObject  — owning Object
    // pButton  — component data (struct Button*)
    // pLeftMouseUp — Node_LeftMouseUpMsgPtr
    SV_PostMessage(hObject, "Click", 0, hObject);
    return TRUE; // handled — stop dispatch
}

// The generated ButtonProc in UIKit_export.c calls:
//   case ID_Node_LeftMouseUp & MSG_DATA_MASK:
//       return Button_LeftMouseUp(object, cmp, wparm, lparm);
```

The `objectProc_t` signature (for reference — use `HANDLER`, not a raw Proc):
```c
LRESULT MyProc(lpObject_t obj, void* cmp, uint32_t msg, wParam_t wParam, lParam_t lParam);
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

## Module XML System

Every C module exposes its API via a `<module>.xml` file co-located with its C sources. The toolchain (`tools/Makefile`) generates:

| Generated file | Template |
|---|---|
| `<module>.h` | `templates/header.php` |
| `<module>_properties.h` | `templates/properties.php` |
| `<module>_export.c` | `templates/export.php` |
| `docs/api/<module>/*.md` | `templates/docs.php` |

Run `make` inside `tools/` to regenerate all files. Run `make docs` for documentation only.

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

Use topics when an interface has more than ~10 methods. See `source/core/core.xml` for a full example with 10 topic separators covering 73 methods.

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

## PHP Code-Generation Templates

The code generation toolchain converts `.xml` module definitions into `.h`, `_properties.h`, and `_export.c` files. Templates are PHP files executed via the `pyphp` Python bridge.

### Running code generation

```bash
# First-time setup (after git submodule update --init --recursive)
pip install -e tools/pyphp

# Regenerate all modules (headers + exports + properties)
cd tools && make

# Regenerate only docs
cd tools && make docs

# Regenerate the orca.dtd UI schema
cd tools && make dtd
```

### Running a single template manually

```bash
cd tools
python3 -m pyphp.pyphp templates/header.php ../source/core/core.xml
python3 -m pyphp.pyphp templates/export.php  ../plugins/UIKit/UIKit.xml
python3 -m pyphp.pyphp templates/docs.php    ../source/core/core.xml
```

### PHP data model (`tools/model/module.php`)

The main class is `Model` (sometimes referred to as `Controller` in older memories — the canonical name is `Model`). Key methods:

```php
$model = new Model($xmlFile);
$model->getInterfaces();  // array of Interface objects
$model->getStructs();     // array of Struct objects
$model->getEnums();       // array of Enum objects
$model->getComponents();  // array of Component (class) objects
$model->getFunctions();   // array of Method objects (global functions)
$model->getEvents();      // array of Event objects
```

`Interface` class key methods:
- `getMethods()` — yields all methods (uses `./methods/method[@name]`); topic separators are transparent
- `getTopics()` — yields `[topicTitle => ["desc" => $desc, "methods" => $methods]]`; iterates `methods/*` children; falls back to single unnamed group if no topics
- `hasTopics()` — returns true if the interface has `<topic>` children inside `<methods>`

### pyphp limitations

The `pyphp` Python-PHP bridge has several quirks:

- **No `(string)$expr`** — use `strval($expr)` instead
- **No `if (!isset($arr[$key]))`** — negated isset in conditions is broken; use: `$val = isset($arr[$key]) ? $arr[$key] : ""; if ($val === "") { … }`
- **No ternary as dict value** — hoist to a variable first: `$ns = $x ? $a : $b; $vars = ["ns" => $ns, …];`
- **No `new $varName()`** — only `new ConcreteClass()` works; same for `$obj->$method()`
- **`?>` eats newlines** — use `echo "…\n"` inside PHP blocks for C code output
- **`config::$Axis`** (static property with `$`) works fine

---

## Key Files for Module XML Work

| File | Role |
|---|---|
| `docs/schemas/module.dtd` | XML schema for module files — update when adding new elements |
| `tools/model/module.php` | PHP data model — add methods to parse new elements |
| `tools/templates/docs.php` | Docs generator — add rendering for new elements |
| `tools/templates/header.php` | C header generator |
| `tools/templates/export.php` | Lua export generator |
| `source/core/core.xml` | Reference example for complex interfaces with topics |

---

## Feature Development Workflow

When implementing a new component, property, message, or any other API addition, always follow this order:

1. **XML first** — Define the public API in the module's `.xml` file (class, properties, messages, handles, methods).
2. **Code generation** — Run `cd tools && make` to regenerate `*.h`, `*_properties.h`, and `*_export.c` from the updated XML.
3. **HANDLER implementations** — Write the hand-coded `.c` handler bodies using the `HANDLER` macro; never edit generated files.
4. **Tests** — Add or update tests (Lua layout tests, C unit tests) and verify with `make test-headless` / `xvfb-run make test`.

> **Never skip step 2.** Editing `.h` or `_export.c` by hand will be overwritten on the next `make`. Commit generated files alongside your XML changes.

---

## Common Tasks

### Add a new element type to the XML schema

1. Add the element to `docs/schemas/module.dtd`
2. Add a parser class or method to `tools/model/module.php`
3. Update `tools/templates/docs.php` to render it
4. Update `docs/MODULE_XML_GUIDE.md` with documentation

### Add a new method to an interface in XML

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

### Module XML — Grouped Container Structure

All child elements in `<class>`, `<struct>`, `<interface>`, and `<message>` are grouped into container elements:
- `<class>`: `<handles>` (with `<handle message="..."/>` children), `<properties>`, `<fields>`, `<methods>`, `<messages>`
- `<struct>`: `<fields>`, `<methods>`
- `<interface>`: `<methods>` (may also contain `<topic>` separators), `<messages>`
- `<message>`: `<fields>`
- `<module>` top-level: `<includes>`, `<externals>`

The PHP parser (`tools/model/module.php`) uses direct-child XPath queries:
- `./properties/property[@name]` (not `.//property[@name]`)
- `./fields/field[@name]` (not `.//field[@name]`)
- `./methods/method[@name]` (not `.//method[@name]`)
- `./messages/message[@name]` (not `.//message[@name]`)
- `handles/handle` (not `handles` with `message` attribute)

### pyphp Template Limitations

When editing PHP templates in `tools/templates/`:

- **No `(string)$expr` casts** — use `strval($expr)` instead.
- **No negated `isset` in `if`** — `if (!isset($arr[$k]))` silently runs the body unconditionally; workaround: `$v = isset($arr[$k]) ? $arr[$k] : ""; if ($v === "") { … }`
- **No ternary as a dict value** — hoist to a variable: `$ns = $x ? $a : $b; $vars = ["ns" => $ns];`
- **No `new $varName()`** — only `new ConcreteClass()` and `$obj->concreteMethod()` work.
- **`?>` eats the following newline** — use `echo "…\n"` inside PHP blocks when emitting C code that must end with a newline.

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

**Anti-pattern 1: Writing C handlers without declaring the class in XML**

If you create `source/core/components/Foo.c` with `HANDLER(Foo, ...)` macros but never add `<class name="Foo">` to the module XML, then:
- `struct Foo` is never generated → the file does not compile
- `FooProc` is never generated → the handlers have no dispatch table
- `REGISTER_CLASS(Foo)` is never generated → the class is never registered
- Message IDs like `ID_Foo_Bar` are never generated → any switch case silently resolves to 0

The fix is always: **write the XML first, run codegen, then write the C**.

**Anti-pattern 2: Declaring the class in XML but not wiring any `<handle>` entries**

A class with no `<handles>` generates an empty `FooProc` switch.  The component exists and can be attached to objects, but **it does nothing** — silently.  Every message that needs a response must have a matching `<handle message="..."/>` in XML and a `HANDLER(Foo, NS, Event)` implementation in C.

**Anti-pattern 3: Declaring and partly implementing a class but not connecting old and new code**

When migrating a global/singleton system into a component:
- The new component struct may exist in XML (generating the class)
- But the old implementation (e.g. a `static` global list) is never replaced
- Result: the component is registered and attachable, but its Proc does nothing; the old global code still runs outside the component lifecycle

Always verify the migration is *complete*: the old code path is deleted, and all behaviour is driven through the new component's message handlers.

**Anti-pattern 4: Including plugin headers from core**

`source/core/` must never `#include <plugins/UIKit/UIKit.h>` or any other plugin header.  The dependency direction is: **plugins depend on core; core does not depend on plugins**.  Violating this creates circular build dependencies, breaks cross-platform builds, and makes the core untestable in isolation.  If core code needs a type from UIKit (e.g. `struct Node`), refactor so the lookup goes through a generic message or a registered accessor.

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
