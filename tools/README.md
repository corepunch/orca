# Module Codegen Architecture

This directory contains the refactored module code generator that processes XML module definitions and generates C headers, Lua bindings, and HTML documentation.

## Architecture

The code generator has been refactored from a monolithic `conv-module.py` into a modular plugin architecture:

```
tools/
├── main.py              # Orchestrator - two-pass XML processing
├── context.py           # Shared state container
├── doc_gen.py           # HTML documentation generation
├── codegen_c.py         # C/Lua code generation utilities
└── plugins/
    ├── struct_plugin.py      # Handles <struct> and <interface> nodes
    ├── component_plugin.py   # Handles <component> nodes
    ├── enum_plugin.py        # Handles <enums> nodes
    └── misc_plugin.py        # Handles <include>, <external>, <resource>, <event>
```

## Usage

```bash
# Generate code for all modules
make modules

# Or use directly
cd tools
python3 main.py \
    ../source/geometry/geom.xml \
    ../source/core/core.xml \
    ../source/renderer/api/renderer.xml \
    ../source/UIKit/UIKit.xml \
    ../source/SceneKit/SceneKit.xml \
    ../source/filesystem/filesystem.xml
```

## Plugin API

Each plugin follows this contract:

```python
def parse(root, node, parser, ctx):
    """
    Args:
        root: XML root element (module)
        node: Current XML node to process
        parser: ParserState with .header, .export, .props file handles
        ctx: Context with registries and output handles
    """
    # Read registries from ctx (read-only)
    # Write output via parser handles
    # Mutate ctx.article for documentation
    pass

def register(parsers):
    """Register tag handlers in the parsers dict"""
    parsers['tag_name'] = parse
```

### Context Object

The `Context` dataclass holds all shared state:

- **Registries** (populated in first pass, read-only in second pass):
  - `structs`: Dict of struct definitions
  - `enums`: Dict of enum definitions
  - `components`: Dict of component definitions
  - `resources`: Dict of resource definitions

- **Output Handles**:
  - `sidebar`: HTML sidebar element
  - `content`: HTML content element
  - `dtd`: DTD file handle
  - `attribs`: List of component attributes

- **Mutable State**:
  - `article`: Current HTML article being generated

## Design Principles

1. **Two-Pass Processing**: First pass populates registries, second pass generates output
2. **No Module Globals**: All state passed explicitly via `Context`
3. **Plugin Isolation**: Plugins don't import from each other
4. **Identical Output**: Refactored code produces byte-for-byte identical output to original

## Files Generated

For each module XML file (e.g., `core.xml`), the generator creates:

- `core.h` - C header with type definitions and function declarations
- `core_export.c` - Lua binding implementation
- `core_properties.h` - Component property enums (if module has components)
- `schemas/orca.dtd` - Accumulated DTD schema
- `../docs/index.html` - Combined HTML documentation

## Migration from Original

The original `conv-module.py` is preserved for reference. The new `main.py` is a drop-in replacement:

- Same command-line interface
- Same input files
- Identical output
- Better maintainability and testability
