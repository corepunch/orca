# Code Generation

ORCA uses a **PHP-based code generation** toolchain to convert XML module definitions into C headers, property hash tables, and Lua binding code. This means that the `.xml` file is the single source of truth for every module's public API — you never edit the generated `.h`, `_properties.h`, or `_export.c` files by hand.

---

## Prerequisites

The toolchain depends on two git submodules:

```bash
# Pull both submodules (one-time after cloning)
git submodule update --init --recursive

# Install the pyphp Python package (one-time)
pip install -e tools/pyphp
```

| Submodule | Path | Purpose |
|---|---|---|
| `platform` | `libs/platform` | Cross-platform abstraction layer |
| `pyphp` | `tools/pyphp` | Python-to-PHP bridge for running the templates |

---

## Running Code Generation

From the `tools/` directory:

```bash
cd tools

make          # Regenerate all modules: headers + export files + property tables
make docs     # Regenerate API documentation Markdown pages
make dtd      # Regenerate docs/schemas/orca.dtd (UI element schema)
```

To regenerate a single template manually:

```bash
cd tools
python3 -m pyphp.pyphp templates/header.php     ../source/core/core.xml
python3 -m pyphp.pyphp templates/properties.php ../source/core/core.xml
python3 -m pyphp.pyphp templates/export.php     ../source/core/core.xml
python3 -m pyphp.pyphp templates/docs.php       ../source/core/core.xml
```

---

## What Gets Generated

For each module listed in `tools/Makefile`, three files are produced:

| Generated file | Template | Contents |
|---|---|---|
| `<module>.h` | `templates/header.php` | Enum typedefs, struct declarations, function prototypes, Lua push/check helpers |
| `<module>_properties.h` | `templates/properties.php` | FNV1a hash constants for every property and event; `GetXxx(obj)` accessor macros |
| `<module>_export.c` | `templates/export.php` | Enum↔string converters, Lua wrapper functions, `luaopen_orca_<module>()` |

When `make docs` is run, one additional file per module is written under `docs/api/`:

| Generated file | Template | Contents |
|---|---|---|
| `docs/api/<module>/README.md` | `templates/docs.php` | Markdown API reference for the module |

---

## Module XML Format

Every module XML file follows this skeleton:

```xml
<?xml version="1.0"?>
<!DOCTYPE module SYSTEM "https://corepunch.github.io/orca/schemas/module.dtd">
<module name="mymodule" namespace="orca" on-luaopen="on_mymodule_registered">

  <!-- Pull in types from another module -->
  <require file="../geometry/geometry.xml"/>

  <!-- Enumerate a C enum -->
  <enums name="Direction">
    <enum name="Horizontal">Left-to-right layout</enum>
    <enum name="Vertical">Top-to-bottom layout</enum>
  </enums>

  <!-- Declare a C struct with Lua bindings -->
  <struct name="MyStruct" export="MyStruct">
    <field name="X" type="float"/>
    <method name="Normalize" lua="true">
      <summary>Normalize the vector in-place.</summary>
    </method>
  </struct>

  <!-- Declare an abstract interface (global functions grouped by prefix) -->
  <interface name="Object" prefix="OBJ_" export="Object">
    <method name="Create" static="true" lua="true">
      <summary>Create a new Object.</summary>
      <returns type="Object" pointer="true"/>
    </method>
  </interface>

  <!-- Declare a component (attaches to Objects) -->
  <class name="MyComponent" export="MyComponent">
    <summary>Moves an object at a configurable speed.</summary>
    <property name="Speed" type="float">Movement speed</property>
  </class>

  <!-- Declare a custom event -->
  <message name="OnValueChanged"/>

</module>
```

See the [Module XML Guide](../MODULE_XML_GUIDE.md) for a complete element reference.

---

## PHP Data Model

The templates access the parsed XML through the `Controller` class defined in `tools/model/module.php`:

```php
$controller = new Controller($argv[1]);  // parse the XML file

$controller->getInterfaces();   // array of Interface objects
$controller->getStructs();      // array of Struct objects
$controller->getEnums();        // array of Enum objects
$controller->getComponents();   // array of Component (class) objects
$controller->getFunctions();    // array of Method objects (global functions)
$controller->getEvents();       // array of Event objects
```

Templates iterate these collections to emit C or Markdown output.

---

## Template Architecture

Templates live in `tools/templates/` and are plain PHP scripts executed by `pyphp`. The entry point is `tools/templates/index.php`, which routes to controller classes in `tools/templates/controllers/`:

| Controller | Actions | Purpose |
|---|---|---|
| `CodegenController` | `header`, `export`, `properties` | Generate C files |
| `DocsController` | `docs` | Generate Markdown API docs |
| `DtdController` | `dtd` | Generate the XML DTD schema |

The header and export outputs are each split into focused sub-templates:

**Header sub-templates** (`tools/templates/header/`):

| Template | Variables | Output |
|---|---|---|
| `preamble.php` | module, namespace, includes, … | `#pragma once`, includes, forward declarations |
| `enums.php` | enums | Enum `typedef` + string converters |
| `declarations.php` | structs, functions, prefix, interfaces | Function prototypes |
| `structs.php` | structs | Struct `typedef` + method stubs |
| `components.php` | components | Component struct + accessor macros |
| `footer.php` | — | Closing guards |

**Export sub-templates** (`tools/templates/`):

| Template | Purpose |
|---|---|
| `export_preamble.php` | `#include` directives |
| `export_enums.php` | Enum ↔ string conversion functions |
| `export_interfaces.php` | Lua wrappers for interface methods |
| `export_structs.php` | Lua wrappers for struct methods |
| `export_components.php` | Component registration + property accessors |
| `export_functions.php` | Wrappers for global functions |
| `export_luaopen.php` | `luaopen_orca_<module>()` function |

---

## pyphp Limitations

The `pyphp` Python-PHP bridge supports most PHP syntax but has a few quirks to be aware of when editing templates:

- **No `(string)$expr`** — use `strval($expr)` instead
- **No negated `isset` in conditions** — `if (!isset($arr[$key]))` silently runs the body unconditionally; workaround: `$v = isset($arr[$k]) ? $arr[$k] : ""; if ($v === "") { … }`
- **No ternary as a dict value** — hoist to a variable first: `$ns = $x ? $a : $b; $vars = ["ns" => $ns];`
- **No `new $varName()`** — only `new ConcreteClass()` works; same restriction applies to `$obj->$method()`
- **`?>` eats the following newline** — use `echo "…\n"` inside PHP blocks when emitting C code that must end with a newline

---

## Key Files

| File | Role |
|---|---|
| `docs/schemas/module.dtd` | XML schema — update when adding new XML elements |
| `tools/model/module.php` | PHP data model — add methods to parse new elements |
| `tools/templates/docs.php` | Docs generator — add rendering for new elements |
| `tools/templates/header.php` | Orchestrates header sub-templates |
| `tools/templates/export.php` | Orchestrates export sub-templates |
| `source/core/core.xml` | Reference example with topics and 70+ methods |
