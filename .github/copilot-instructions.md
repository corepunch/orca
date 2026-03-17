# ORCA — Copilot Instructions

This file gives GitHub Copilot (and any AI coding assistant) the essential context needed to work effectively in the ORCA repository without having to re-discover it from scratch.

---

## Repository Layout

```
orca/
├── source/          # C engine source: core/, geometry/, renderer/, filesystem/
├── plugins/         # Plugin modules: UIKit/, SceneKit/, SpriteKit/, DarkReign/
├── include/         # Public C headers (orca.h, renderer.h, …)
├── docs/            # MkDocs website source (Markdown + schemas)
│   ├── MODULE_XML_GUIDE.md   # How to write module XML files
│   └── schemas/    # DTD schema files for XML validation
├── tools/           # Code-generation toolchain
│   ├── model/       # PHP data model (module.php — XML parser)
│   ├── templates/   # PHP code-gen templates (header, export, docs, …)
│   ├── Makefile     # `make all` → headers+exports; `make docs` → Markdown docs
│   └── pyphp/       # Python-to-PHP bridge (empty dir — pyphp installed via pip)
├── mkdocs.yml       # MkDocs site configuration
└── samples/         # Example projects
```

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

Templates live in `tools/templates/` and are plain PHP files. They are run with:

```bash
cd tools
php templates/header.php ../source/core/core.xml
```

Or using the `pyphp` Python bridge (installed via `pip install pyphp`):

```bash
cd tools
python3 /path/to/pyphp.py templates/header.php ../source/core/core.xml
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

## Plugin System

Plugins use `ClassDesc` with `SuperClassID` (category) and `ClassData` (interface ptr) fields. `SCLASS_*` constants in `include/plugapi.h`. Use `OBJ_EnumClassesBySuperClass()` to find plugins by category.

Plugin proc convention: `ClassDesc.ObjProc` handles message IDs (`kMsg*` from `plugapi.h`).

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
