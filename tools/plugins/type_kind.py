"""Type-kind resolution for codegen dispatch.

``resolve(type_name)`` returns a ``Kind`` enum value so every dispatch site
can index a table or do a single comparison instead of repeating the same
``if/elif`` chain.
"""

from enum import Enum

from . import Workspace, atomic_types


class Kind(Enum):
    ATOMIC    = "atomic"
    ENUM      = "enum"
    STRUCT    = "struct"
    COMPONENT = "component"
    RESOURCE  = "resource"
    FIXED     = "fixed"
    BOOL      = "bool"
    UNKNOWN   = "unknown"


def resolve(type_name: str) -> Kind:
    """Return the ``Kind`` for *type_name*.

    Checked in priority order so that the special names ``"fixed"`` and
    ``"bool"`` (which also appear in ``atomic_types``) are classified as
    ``Kind.FIXED`` / ``Kind.BOOL`` rather than ``Kind.ATOMIC``.
    """
    if type_name == "fixed":               return Kind.FIXED
    if type_name == "bool":                return Kind.BOOL
    if type_name in atomic_types:          return Kind.ATOMIC
    if type_name in Workspace.enums:       return Kind.ENUM
    if type_name in Workspace.structs:     return Kind.STRUCT
    if type_name in Workspace.components:  return Kind.COMPONENT
    if type_name in Workspace.resources:   return Kind.RESOURCE
    return Kind.UNKNOWN
