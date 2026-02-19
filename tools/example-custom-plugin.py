#!/usr/bin/env python3
"""
Example: Custom Plugin for conv-module.py

This example demonstrates how to create a custom output plugin that generates
JSON metadata files from XML module definitions.

Usage:
    python3 example-custom-plugin.py ../source/geometry/geom.xml
"""

import sys
import os
import json

# Add the tools directory to the path so we can import from conv-module
sys.path.insert(0, os.path.dirname(__file__))

# Import necessary components from conv-module
# We need to import it properly to access the classes
import importlib.util
spec = importlib.util.spec_from_file_location("conv_module", "conv-module.py")
conv_module = importlib.util.module_from_spec(spec)
sys.modules["conv_module"] = conv_module
spec.loader.exec_module(conv_module)

# Now we can access the classes
BasePlugin = conv_module.BasePlugin
OutputPlugin = conv_module.OutputPlugin
PluginRegistry = conv_module.PluginRegistry

class JsonPlugin(OutputPlugin):
    """Plugin for generating JSON metadata files (.json)"""
    def __init__(self, filename):
        super().__init__(filename, ".json")
        self.metadata = {
            "module": None,
            "namespace": None,
            "structs": [],
            "enums": [],
            "components": [],
            "functions": []
        }
    
    def set_module_info(self, name, namespace):
        """Set module basic information"""
        self.metadata["module"] = name
        self.metadata["namespace"] = namespace
    
    def add_struct(self, name, fields):
        """Add a struct to metadata"""
        self.metadata["structs"].append({
            "name": name,
            "fields": fields
        })
    
    def add_enum(self, name, values):
        """Add an enum to metadata"""
        self.metadata["enums"].append({
            "name": name,
            "values": values
        })
    
    def add_component(self, name, properties):
        """Add a component to metadata"""
        self.metadata["components"].append({
            "name": name,
            "properties": properties
        })
    
    def add_function(self, name, args, returns):
        """Add a function to metadata"""
        self.metadata["functions"].append({
            "name": name,
            "arguments": args,
            "returns": returns
        })
    
    def open(self):
        """Override open - we don't write until close, so return None"""
        # We'll write JSON at close time, so no file handle needed during open
        return None
    
    def close(self):
        """Write the JSON metadata to file"""
        output_filename = self.filename.replace(".xml", ".json")
        with open(output_filename, "w") as f:
            json.dump(self.metadata, f, indent=2)
        print(f"Generated JSON metadata: {output_filename}")

def main():
    """Main function demonstrating custom plugin usage"""
    if len(sys.argv) < 2:
        print("Usage: python3 example-custom-plugin.py <module.xml>")
        sys.exit(1)
    
    # Register the custom JSON plugin
    PluginRegistry.register('json', JsonPlugin)
    
    # Example: You could disable HTML generation if you only want specific outputs
    # PluginRegistry.disable_plugin('html')
    
    print("Registered plugins:", list(PluginRegistry._plugins.keys()))
    print("Enabled plugins:", PluginRegistry.get_enabled_plugins())
    
    # Now process the XML file(s)
    # Note: This is a simplified example. In practice, you'd need to integrate
    # the JSON plugin with the actual parsing logic in conv-module.py
    
    # Create a JSON plugin instance for demonstration
    json_plugin = JsonPlugin(sys.argv[1])
    json_plugin.set_module_info("example_module", "orca")
    json_plugin.add_struct("Vector2D", ["x", "y"])
    json_plugin.add_enum("RotationOrder", ["XYZ", "XZY", "YZX"])
    json_plugin.close()
    
    print("\nCustom plugin demonstration complete!")
    print("Note: For full integration, you would need to modify the parser")
    print("functions in conv-module.py to call your plugin's methods.")

if __name__ == "__main__":
    main()
