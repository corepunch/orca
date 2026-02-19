"""Shared mutable state used by all plugins.

The dictionaries are populated by conv-module.py as XML files are parsed
and are then available to every plugin via import.

Because these are *dict objects* (mutable), importing a name from this
module gives plugins a live reference – any .update() call made by the
main script is immediately visible to all plugins.
"""

g_structs: dict = {}
g_enums: dict = {}
g_components: dict = {}
g_resources: dict = {}
