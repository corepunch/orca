import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from typing import Dict, List

@dataclass
class Context:
	"""Context holding all shared state passed explicitly to plugins."""
	# Registries
	structs: Dict[str, ET.Element] = field(default_factory=dict)
	enums: Dict[str, ET.Element] = field(default_factory=dict)
	components: Dict[str, ET.Element] = field(default_factory=dict)
	resources: Dict[str, ET.Element] = field(default_factory=dict)
	
	# Output handles
	sidebar: ET.Element = None
	content: ET.Element = None
	dtd: object = None  # file handle
	attribs: List[str] = field(default_factory=list)
	
	# Current article reference (mutated by doc-generating plugins)
	article: ET.Element = None
