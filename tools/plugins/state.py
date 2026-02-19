"""Shared mutable state used by all plugins.

``Workspace`` is a simple namespace class whose *class attributes* are the
live dicts populated by conv-module.py as XML files are parsed.  Because the
attributes are mutable dict objects, any plugin that imports ``Workspace``
always sees the up-to-date contents – no re-import or passing-around required.

Example usage inside a plugin::

    from .state import Workspace

    struct = Workspace.structs.get(type_name)
    if type_name in Workspace.enums:
        ...
"""


class Workspace:
    """Central namespace for all shared parse-time state."""

    structs:    dict = {}   #: All ``<struct>`` elements, keyed by name
    enums:      dict = {}   #: All ``<enums>`` elements, keyed by name
    components: dict = {}   #: All ``<component>`` elements, keyed by name
    resources:  dict = {}   #: All ``<resource>`` elements, keyed by type
