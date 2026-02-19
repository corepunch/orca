# Refactoring Notes: conv-module.py → Plugin Architecture

## Overview
Successfully refactored monolithic `conv-module.py` (979 lines) into a modular plugin architecture while maintaining 100% output compatibility.

## File Structure

### Before (1 file, 979 lines)
```
tools/
└── conv-module.py  # Monolithic file with all logic
```

### After (9 files, modular)
```
tools/
├── main.py                    # Entry point & orchestrator
├── context.py                 # Shared state container
├── doc_gen.py                 # HTML documentation utilities
├── codegen_c.py               # C/Lua code generation utilities
├── plugins/
│   ├── __init__.py
│   ├── struct_plugin.py       # Struct/interface/function handlers
│   ├── component_plugin.py    # Component handler
│   ├── enum_plugin.py         # Enum handler
│   └── misc_plugin.py         # Include/external/resource/event handlers
└── README.md                  # Architecture documentation
```

## Key Changes

### 1. Eliminated Module-Level Globals
**Before:**
- 13 global variables (g_structs, g_enums, g_components, etc.)
- Direct access throughout codebase
- Difficult to test and reason about

**After:**
- All state in `Context` dataclass
- Explicitly passed to plugins
- Clear data flow

### 2. Plugin Isolation
**Before:**
- All parsers in single file sharing state
- Functions calling each other directly
- Tight coupling

**After:**
- Each plugin in separate file
- Clean API contract: `parse(root, node, parser, ctx)`
- No cross-plugin imports
- Independently testable

### 3. Two-Pass Architecture
**Before:**
- Registries populated during parsing
- Inconsistent access patterns

**After:**
- **Pass 1**: Populate all registries
- **Pass 2**: Generate output using complete registries
- Clearer separation of concerns

### 4. Build System Integration
Updated both build systems to use new entry point:
- `Makefile`: `python3 main.py ...`
- `premake5.lua`: `python3 tools/main.py ...`

## Plugin Registration

Each plugin exports a `register(parsers)` function:

```python
def register(parsers):
    parsers['struct'] = parse
    parsers['interface'] = parse
```

In `main.py`:
```python
parsers = {}
struct_plugin.register(parsers)
component_plugin.register(parsers)
enum_plugin.register(parsers)
misc_plugin.register(parsers)
```

## Verification

All outputs verified to be byte-for-byte identical:
- ✅ C headers (.h files)
- ✅ Lua exports (_export.c files)
- ✅ Properties headers (_properties.h files)
- ✅ DTD schema (orca.dtd)
- ✅ HTML documentation (index.html)

## Benefits

1. **Maintainability**: Each plugin is ~200 lines vs 979-line monolith
2. **Testability**: Plugins can be tested in isolation
3. **Extensibility**: New node types → new plugin file
4. **Readability**: Clear separation of concerns
5. **Documentation**: Each module has focused responsibility

## Migration Path

The original `conv-module.py` is preserved for reference. The new architecture is a drop-in replacement with identical behavior.

## Future Enhancements

Potential improvements enabled by this refactoring:
- Unit tests for individual plugins
- Type hints throughout
- Plugin versioning system
- Parallel processing of independent modules
- Plugin-specific configuration
