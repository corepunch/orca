# Code Generation

ORCA uses a small C code generation host to convert `.cgen` module definitions into C headers, property hash tables, and Lua binding code. The `.cgen` file is the single source of truth for each module's public API, so generated `.h`, `_properties.h`, and `_export.c` files should not be edited by hand.

---

## Prerequisites

After cloning, initialize submodules before building:

```bash
git submodule update --init --recursive
```

The code generator is built from the main `Makefile` and requires libxml2 development headers through `pkg-config`.

---

## Running Code Generation

From the repository root:

```bash
make modules
```

The `modules` target builds `build/bin/cgen`, builds every `tools/codegen/plugins/*.c` file into `build/plugins/codegen/lib*.so`, discovers module files with `find source plugins -name '*.cgen'`, and regenerates the three committed generated files for each module.

To run one generator manually, use the root-relative stdout form:

```bash
make codegen-host
build/bin/cgen source/core/core.cgen build/plugins/codegen/libheader.so > generated/core/core.h
build/bin/cgen source/core/core.cgen build/plugins/codegen/libproperties.so > generated/core/core_properties.h
build/bin/cgen source/core/core.cgen build/plugins/codegen/libexport.so > generated/core/core_export.c
```

`cgen` also accepts `-o <file>` for scripts that prefer explicit output paths.

---

## What Gets Generated

For each `.cgen` module under `source/` or `plugins/`, three files are produced:

| Generated file | Generator plugin | Contents |
|---|---|---|
| `<module>.h` | `libheader.so` | Enum typedefs, struct declarations, function prototypes, Lua push/check helpers |
| `<module>_properties.h` | `libproperties.so` | FNV1a hash constants for every property and event; `GetXxx(obj)` accessor macros |
| `<module>_export.c` | `libexport.so` | Enum/string converters, Lua wrapper functions, `luaopen_orca_<module>()` |

---

## Module Codegen Format

Every module `.cgen` file is XML and follows this skeleton:

```xml
<?xml version="1.0"?>
<!DOCTYPE module SYSTEM "https://corepunch.github.io/orca/schemas/module.dtd">
<module name="mymodule" namespace="orca" on-luaopen="on_mymodule_registered">

  <require file="../geometry/geometry.cgen"/>

  <enums>
    <enum name="Direction">
      <value name="Horizontal">Left-to-right layout</value>
      <value name="Vertical">Top-to-bottom layout</value>
    </enum>
  </enums>

  <struct name="MyStruct">
    <fields>
      <field name="X" type="float"/>
    </fields>
    <methods>
      <method name="Normalize" lua="true">
        <summary>Normalize the vector in-place.</summary>
      </method>
    </methods>
  </struct>

  <interface name="Object" prefix="OBJ_" export="Object">
    <methods>
      <method name="Create" static="true" lua="true">
        <summary>Create a new Object.</summary>
        <returns type="Object" pointer="true"/>
      </method>
    </methods>
  </interface>

  <class name="MyComponent" >
    <summary>Moves an object at a configurable speed.</summary>
    <handles>
      <handle message="Object.Start"/>
      <handle message="Object.Animate"/>
    </handles>
    <properties>
      <property name="Speed" type="float" default="1.0">Movement speed</property>
    </properties>
    <messages>
      <message name="OnValueChanged" routing="Direct"/>
    </messages>
  </class>

</module>
```

See the [Module Codegen Guide](../MODULE_XML_GUIDE.md) for a complete element reference.

---

## C Generator Architecture

The host executable in `tools/codegen/src/` loads a `.cgen` file with libxml2, builds a flat model with parent IDs, loads a generator plugin with `dlopen`, and asks the plugin to write output.

| File | Role |
|---|---|
| `tools/codegen/src/model.c` | Parser/model loader for `.cgen` files |
| `tools/codegen/src/main.c` | CLI, plugin loading, stdout/`-o` output selection |
| `tools/codegen/include/cg_api.h` | Stable host/plugin API |
| `tools/codegen/plugins/header.c` | C header generator |
| `tools/codegen/plugins/properties.c` | Property/hash header generator |
| `tools/codegen/plugins/export.c` | Lua export generator |
| `source/core/core.cgen` | Reference example with topics and 70+ methods |

Generator plugins should use `cg_foreach(model, parent_id, kind, node)` for direct children instead of scanning descendants unless they explicitly need dependency lookup.
