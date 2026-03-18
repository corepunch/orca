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
