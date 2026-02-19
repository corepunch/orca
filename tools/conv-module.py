"""conv-module.py – XML module definition to code/documentation converter.

Architecture
------------
All output formats are implemented as *plugins* – subclasses of
``plugins.Plugin`` defined in their own files under ``tools/plugins/``.
Each plugin is auto-registered when its module is imported, so adding a
new output format only requires:

1. Creating a file in ``tools/plugins/`` that defines a ``Plugin`` subclass.
2. Adding an import below (or discovering the file dynamically).

No manual registration call is needed; the base-class metaclass hook
(``Plugin.__init_subclass__``) takes care of everything.

See ``tools/plugins/__init__.py`` for the plugin base class and registry,
and the individual plugin files for concrete examples.
"""

import sys
import os
import xml.etree.ElementTree as ET

# ---------------------------------------------------------------------------
# Plugin system – import loads & auto-registers each plugin
# ---------------------------------------------------------------------------
import plugins                   # exposes Plugin base class and get_plugins()
import plugins.dtd_writer        # DTDWriter
import plugins.header_writer     # HeaderWriter
import plugins.html_writer       # HtmlWriter
import plugins.properties_writer # PropertiesWriter
import plugins.export_writer     # ExportWriter

# ---------------------------------------------------------------------------
# Shared mutable state (populated while parsing, visible to all plugins)
# ---------------------------------------------------------------------------
from plugins.state import g_structs, g_enums, g_components, g_resources


# ---------------------------------------------------------------------------
# Core parsing logic
# ---------------------------------------------------------------------------

def call_plugin_hook(hook, *args):
    """Call *hook* on every registered plugin that implements it."""
    for plugin in plugins.get_plugins():
        method = getattr(plugin, hook, None)
        if method:
            method(*args)


def read_xml(filename):
    if not os.path.exists(filename):
        print(f"File not found: {filename}")
        return

    tree = ET.parse(filename)
    root = tree.getroot()

    if root.tag != "module":
        print("Expected 'module' tag")
        return

    call_plugin_hook('open', filename, root)

    g_structs.update({s.get('name'): s for s in root.findall(".//struct[@name]")})
    g_enums.update({e.get('name'): e for e in root.findall(".//enums[@name]")})
    g_components.update({c.get('name'): c for c in root.findall(".//component[@name]")})
    g_resources.update({c.get('type'): c for c in root.findall(".//resource[@type]")})

    for struct in (
        root.findall('struct') + root.findall('interface') + root.findall('component')
    ):
        struct_name = struct.get('name')
        call_plugin_hook("struct_fwd_def", struct_name)
        call_plugin_hook("lua_accessors", struct_name)

    for node in root:
        call_plugin_hook(f"on_{node.tag}", root, node)

    call_plugin_hook('close')


if __name__ == "__main__":
    for i in range(len(sys.argv) - 1):
        read_xml(sys.argv[i + 1])

    call_plugin_hook('finalize')
