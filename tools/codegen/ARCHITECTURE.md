# Codegen — Architecture

## Overview

`codegen` is a plugin-driven code generation framework that parses `.cgen` XML files and produces output via dynamically loaded plugins. The core binary (`cgen`) knows nothing about output formats — all format-specific logic lives in `.so` plugins.

## Pipeline

```
.cgen XML → model.c (parse) → cg_model flat array → dlopen plugin → plugin->emit(host, model, output) → files
```

1. **CLI parsing** (`main.c:16`) — extracts xml path, plugin path, output path
2. **Model loading** (`model.c:280`) — parses XML via libxml2, walks tree into flat `cg_node[]`, recursively loads `<require>`d files, sets module metadata
3. **Plugin loading** (`main.c:60`) — `dlopen()` + `dlsym("cg_plugin_init_v1")`, validates ABI version
4. **Emit** (`main.c:83`) — calls `plugin->emit(&host, &model, output_path)`
5. **Cleanup** (`main.c:84`) — `dlclose()` + `cg_model_free()`

## Parsing (`src/model.c`)

1. `xmlReadFile()` loads the `.cgen` XML document
2. `walk()` recursively walks the XML tree, mapping each element to a `cg_node` via `elem_kind()`
3. XML attributes are mapped to `name`, `type`, `extra`, `aux`, `aux2`, and flags per `cg_kind` semantics
4. `<summary>` elements become `doc` strings
5. `<require file="..."/>` triggers recursive parsing of referenced files, with nodes appended to the same flat array
6. Module-level metadata (`module_name`, `module_ns`, `prefix`, `on_luaopen`, `after_luaopen`) is extracted from the root `<module>` element

## File Layout

| File | Purpose |
|---|---|
| `include/cg_api.h` | Core API: `cg_kind`, `CG_FLAG_*`, `cg_node`, `cg_model`, `cg_host_v1`, `cg_plugin_v1`, iteration helpers |
| `include/outbuf.h` | Dynamic string buffer (`outbuf`) with `ob_grow()` and `ob_printf()` |
| `src/main.c` | CLI entry point, argument parsing, plugin loading, emit invocation |
| `src/model.c` | XML parsing into `cg_model`, recursive requirement loading |
| `src/model.h` | `cg_model_load()` / `cg_model_free()` declarations |
| `src/util.c` | `cg_fnv1a32()`, `cg_write_file()`, `cg_logf()` |
| `src/util.h` | Utility declarations |
| `plugins/` | Plugin sources: `header.c`, `properties.c`, `docs.c`, `dtd.c`, `export.c`, `header_stub.c` |
| `Makefile` | Build targets: `codegen-host`, `run-sample`, `run-sample-properties`, `run-sample-docs`, `run-sample-dtd` |

## Plugins

| Plugin source | Generates | Output pattern |
|---|---|---|
| `plugins/header.c` → `libheader.so` | C headers (`.h`) | `out/…/<Module>.h` |
| `plugins/properties.c` → `libproperties.so` | Property accessors (`.h`) | `out/…/<Module>_properties.h` |
| `plugins/export.c` → `libexport.so` | Lua export glue (`.c`) | `out/…/<Module>_export.c` |
| `plugins/docs.c` → `libdocs.so` | Documentation (`.md`) | — |
| `plugins/dtd.c` → `libdtd.so` | DTD for UI definitions (`.dtd`) | — |
| `plugins/header_stub.c` → `libheader_stub.so` | Minimal header stubs | — |

Built to `build/plugins/codegen/`.
