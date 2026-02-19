# Conv-Module Plugin Architecture

The `conv-module.py` tool has been refactored to use a plugin-based architecture for generating different output formats from XML module definitions.

## Overview

The plugin architecture allows you to:
- Generate multiple output formats from a single XML definition
- Easily add new output formats without modifying core logic
- Enable/disable specific output formats as needed
- Create custom plugins for project-specific needs

## Built-in Plugins

| Plugin | Extension | Description |
|--------|-----------|-------------|
| `HeaderPlugin` | `.h` | Generates C header files with type definitions and function declarations |
| `ExportPlugin` | `_export.c` | Generates C source files with Lua bindings |
| `PropsPlugin` | `_properties.h` | Generates component property metadata |
| `HtmlPlugin` | N/A | Generates HTML documentation (writes to `../docs/index.html`) |

## Usage

### Basic Usage

```bash
python3 conv-module.py <module.xml> [module2.xml ...]
```

This will generate all enabled outputs for each XML module file.

### Programmatic Usage

```python
from conv_module import PluginRegistry

# Disable HTML generation
PluginRegistry.disable_plugin('html')

# Enable only specific plugins
PluginRegistry.disable_plugin('export')
PluginRegistry.disable_plugin('props')
# Now only header and html will be generated
```

## Creating Custom Plugins

### File-Based Output Plugin

For plugins that generate files (like C, JSON, etc.):

```python
from conv_module import OutputPlugin, PluginRegistry

class MyCustomPlugin(OutputPlugin):
    """Plugin for generating custom format files"""
    def __init__(self, filename):
        super().__init__(filename, ".mycustom")
    
    def write_header(self, text):
        """Custom method for your plugin"""
        self.write(f"# {text}")

# Register the plugin
PluginRegistry.register('mycustom', MyCustomPlugin)
```

### Non-File Output Plugin

For plugins that don't write to individual files:

```python
class CustomPlugin:
    """Plugin for custom processing"""
    def __init__(self):
        self.data = []
    
    def process(self, element):
        """Process XML elements"""
        self.data.append(element)
    
    def finalize(self):
        """Called when processing is complete"""
        # Do something with collected data
        pass

# Register the plugin
PluginRegistry.register('custom', CustomPlugin)
```

## Plugin Registry API

### Register a Plugin

```python
PluginRegistry.register(name: str, plugin_class: type)
```

### Unregister a Plugin

```python
PluginRegistry.unregister(name: str)
```

### Enable/Disable Plugins

```python
PluginRegistry.enable_plugin(name: str)
PluginRegistry.disable_plugin(name: str)
```

### Get Plugin Information

```python
# Get list of enabled plugins
enabled = PluginRegistry.get_enabled_plugins()

# Get plugin class
plugin_class = PluginRegistry.get_plugin_class(name: str)

# Create plugin instance
plugin = PluginRegistry.create_plugin(name: str, *args, **kwargs)
```

## Example: JSON Metadata Plugin

See `example-custom-plugin.py` for a complete example of creating a custom JSON output plugin.

```bash
python3 example-custom-plugin.py ../source/geometry/geom.xml
```

This demonstrates:
- Creating a custom plugin class
- Registering it with the PluginRegistry
- Using the plugin to generate output

## Architecture

```
XML Module Definition
        |
        v
   read_xml()
        |
        v
  ParserState (creates plugins via PluginRegistry)
        |
        +-- HeaderPlugin → .h
        +-- ExportPlugin → _export.c
        +-- PropsPlugin → _properties.h
        +-- HtmlPlugin → HTML docs
        +-- [Your Custom Plugin] → custom output
```

## Extending the System

To fully integrate a custom plugin with the parsing system, you would need to:

1. Create your plugin class
2. Register it with PluginRegistry
3. Modify the relevant parser functions (e.g., `struct_parse`, `function_parse`) to call your plugin's methods
4. Add your plugin initialization in `ParserState.__new__`

The current architecture provides the foundation for this extensibility while maintaining backward compatibility with existing code.
