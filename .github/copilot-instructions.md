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

### Object (`source/core/object.c`)

Every scene element is an `Object`. Objects form a parent→children tree and carry:

- A name, class name, and source file
- A component linked-list (see below)
- A flat byte buffer (`data[]`) for property storage
- A Lua state pointer for scripting

### Components (`source/core/component.c`)

A component is a block of typed data + a message-handler function attached to an Object. Each component is described by a `ClassDesc` (`include/orca.h`):

```c
struct ClassDesc {
  objectProc_t ObjProc;        // Message handler — dispatches kMsg* events
  lpcPropertyType_t Properties;// Sorted array of property descriptors
  lpcString_t ClassName;       // Human-readable name (e.g. "Button")
  uint32_t ParentClasses[16];  // Inherited class IDs (inheritance chain)
  uint32_t NumProperties;      // Number of directly owned properties
  uint32_t ClassID;            // FNV1a hash of ClassName
  uint32_t ClassSize;          // Size of instance data
  void const *Defaults;        // Default property values struct
};
```

Call `OBJ_RegisterClass(&desc)` at module init to register a component type. Call `OBJ_AddComponent(obj, classID)` to attach it to an object at runtime.

### How to add a new component type

1. **Define the component in XML** (in the module's `.xml` file):
   ```xml
   <class name="MyComponent" export="MyComponent">
     <summary>What this component does.</summary>
     <property name="Speed" type="float">Movement speed</property>
   </class>
   ```

2. **Run the code generator** to regenerate the header, properties, and export files:
   ```bash
   cd tools && make
   ```

3. **Write the C implementation** (e.g. `source/mymodule/mycomponent.c`):
   ```c
   #include "mymodule_properties.h"
   
   struct MyComponent { float Speed; };
   
   static LRESULT MyComponent_Proc(lpObject_t obj, uint32_t msg,
                                    wParam_t wParam, lParam_t lParam) {
     struct MyComponent *self = GetMyComponent(obj);
     switch (msg) {
       case kEventCreate: self->Speed = 1.0f; break;
     }
     return 0;
   }
   
   static struct MyComponent defaults = { .Speed = 1.0f };
   
   static ClassDesc MyComponentClass = {
     .ObjProc       = MyComponent_Proc,
     .ClassName     = "MyComponent",
     .ClassID       = ID_MyComponent,   // generated hash
     .ClassSize     = sizeof(struct MyComponent),
     .Defaults      = &defaults,
   };
   ```

4. **Register at module init**:
   ```c
   OBJ_RegisterClass(&MyComponentClass);
   ```

5. **Use from Lua** (bindings are auto-generated):
   ```lua
   local obj = orca.create("MyComponent")
   obj.Speed = 5.0
   ```

---

## Plugin System

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

**Message handling** — each component's `ObjProc` receives messages defined in `source/core/core_properties.h`:

```c
LRESULT Button_Proc(lpObject_t obj, uint32_t msg, wParam_t wParam, lParam_t lParam) {
  struct Button *self = GetButton(obj);
  switch (msg) {
    case kEventCreate:   /* initialize */ break;
    case kEventDestroy:  /* cleanup    */ break;
    case kEventDraw:     /* render     */ break;
    case kMsgMouseUp:    /* input      */ break;
  }
  return 0;
}
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
| `<struct>` | C struct with fields and methods |
| `<interface>` | Abstract interface (like a base class) |
| `<class>` | Component (attaches to Objects) |
| `<function>` | Global function |
| `<event>` | Event declaration |
| `<method>` | Method on a struct/interface/class |
| `<topic>` | Inline section separator **inside an `<interface>`** — has `title` attr and description text (docs only) |
| `<field>` | Struct data member |
| `<property>` | Component property (exposed to Lua) |
| `<arg>` | Function/method argument |
| `<returns>` | Return value description |

### `<topic>` — inline section separator inside interfaces

`<topic>` is a **sibling separator** placed between `<method>` elements, not a container wrapping them. It marks the start of a named section in generated documentation, with optional prose description as its text content.

```xml
<interface name="Object" prefix="OBJ_" export="Object" no-check="true">
  <summary>Core engine host object.</summary>

  <topic title="Lifecycle">Manages object creation, initialization, and destruction.</topic>
  <method name="Awake" lua="true">
    <summary>Initializes the object when loaded.</summary>
  </method>

  <topic title="Hierarchy">Navigates and manipulates the parent-child relationship tree.</topic>
  <method name="AddChild">
    <summary>Add a child object.</summary>
    <arg name="child" type="Object" pointer="true">Child to add</arg>
  </method>
</interface>
```

- `title` attribute (required) — the section heading in generated docs
- Text content (optional) — description paragraph rendered below the heading
- Methods **after** a `<topic>` belong to that section until the next `<topic>` or end of interface
- `<topic>` is invisible to code generation: `getMethods()` (XPath `.//method[@name]`) traverses all sibling methods regardless of intervening topic separators

Use topics when an interface has more than ~10 methods. See `source/core/core.xml` for a full example with 10 topic separators covering 73 methods.

The DTD allows `(method | topic)*` as the `interface` content model. `struct` and `class` elements do **not** support topics.

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
- `getMethods()` — yields all methods (uses `.//method[@name]`); topic separators are transparent
- `getTopics()` — yields `[topicTitle => ["desc" => $desc, "methods" => $methods]]`; falls back to a single unnamed group if no topics defined
- `hasTopics()` — returns true if the interface has `<topic>` children

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

## Common Tasks

### Add a new element type to the XML schema

1. Add the element to `docs/schemas/module.dtd`
2. Add a parser class or method to `tools/model/module.php`
3. Update `tools/templates/docs.php` to render it
4. Update `docs/MODULE_XML_GUIDE.md` with documentation

### Add a new method to an interface in XML

```xml
<method name="MyMethod" export="myMethod" lua="true">
  <summary>One-line description.</summary>
  <details>Detailed explanation.</details>
  <arg name="value" type="int">The value to pass</arg>
  <returns type="bool">True on success</returns>
</method>
```

If the interface uses `<topic>` separators, place the method after the appropriate `<topic title="...">` separator.

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

### Module XML — Topics Are Siblings, Not Containers

`<topic>` elements are **sibling separators** between `<method>` elements inside an `<interface>` — they are NOT containers. Methods follow a `<topic>` as siblings until the next `<topic>` or end of interface. `getMethods()` uses `.//method[@name]` and traverses all methods regardless of intervening topics. Topics are invisible to code generation; they only affect generated documentation.

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

`renderer_Init()` (`source/renderer/r_main.c`) is the only function that creates the OpenGL context and calls `R_InitBuffers()` → `glGenBuffers()`, which sets `tr.buffer` to a non-zero value. Requiring `orca.renderer` (via `luaopen_orca_renderer`) only calls `WI_Init()` — it does **not** initialize the full renderer.

**Gotcha:** Calling `renderer_Shutdown()` without having first called `renderer_Init()` crashes in `WI_MakeCurrentContext()` because no GL context exists. The guard `if (!tr.buffer) return;` at the top of `renderer_Shutdown` short-circuits this path. Always check `tr.buffer` before performing any GL teardown.

This matters in test harnesses (`RunTest` in `source/orca.c`) that `require "orca.renderer"` for XML parsing but never create a window.

### XML Attribute Iterator — `xmlGetProp` vs `xmlNodeListGetString`

The XML attribute iterator in `source/parsers/p_xml.c` previously used `xmlNodeListGetString(attr->parent->doc, attr->children, 1)` to read attribute values. When `XML_PARSE_COMPACT` is active (the default flag in `include/orcadef.h`), libxml2 may store small text nodes differently, causing `xmlNodeListGetString` to return `NULL` even for non-empty attributes. This manifested as `setName(nil)` errors in `file-xml.lua`.

**Fix:** Use `xmlGetProp(node, attr->name)` (or `xmlGetNsProp` for namespaced attributes) — the canonical libxml2 API for reading attribute values, which is immune to compact-mode storage differences. Always free the returned `xmlChar*` with `xmlFree`.

### Screen ResizeMode in Tests and Headless Contexts

`Screen.MeasureOverride` calls `WI_GetSize()` and overrides the screen's requested `Width`/`Height` with the actual window dimensions when `ResizeMode == kResizeModeCanResize` (the **default** value). On a virtual framebuffer (e.g., `xvfb-run` in CI), the window is 640×480, so any test that creates a `Screen { Width = 1000, Height = 1000 }` without setting `ResizeMode` will silently get a 640×480 screen — causing layout assertions to fail.

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
