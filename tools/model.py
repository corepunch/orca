import os, re
import xml.etree.ElementTree as ET
from enum import Enum

class Kind(Enum):
    ATOMIC    = "atomic"
    ENUM      = "enum"
    STRUCT    = "struct"
    COMPONENT = "component"
    RESOURCE  = "resource"
    FIXED     = "fixed"
    UNKNOWN   = "unknown"

Axis = [
	(re.compile(r"(.+)\.Axis\[0\]\.Left(.*)"),              r"\1Left\2"),
	(re.compile(r"(.+)\.Axis\[0\]\.Right(.*)"),             r"\1Right\2"),
	(re.compile(r"(.+)\.Axis\[1\]\.Left(.*)"),              r"\1Top\2"),
	(re.compile(r"(.+)\.Axis\[1\]\.Right(.*)"),             r"\1Bottom\2"),
	(re.compile(r"(.+)\.Axis\[2\]\.Left(.*)"),              r"\1Front\2"),
	(re.compile(r"(.+)\.Axis\[2\]\.Right(.*)"),             r"\1Back\2"),
	(re.compile(r"Size.Axis\[0\]\.(?:Requested)?(.*)"),     r"\1Width"),
	(re.compile(r"Size.Axis\[1\]\.(?:Requested)?(.*)"),     r"\1Height"),
	(re.compile(r"Size.Axis\[2\]\.(?:Requested)?(.*)"),     r"\1Depth"),
	(re.compile(r"(.+)\.Axis\[0\]"),                        r"Horizontal\1"),
	(re.compile(r"(.+)\.Axis\[1\]"),                        r"Vertical\1"),
	(re.compile(r"(.+)\.Axis\[2\]"),                        r"Depth\1"),
	(re.compile(r"Border\.Radius\.(.+)Radius"),             r"Border\1Radius"),
]

atomic_types = {
	"float":  ("luaL_checknumber",  "lua_pushnumber"),
	"int":    ("luaL_checknumber",  "lua_pushnumber"),
	"uint":   ("luaL_checknumber",  "lua_pushnumber"),
	"long":   ("luaL_checkinteger", "lua_pushinteger"),
	"bool":   ("lua_toboolean",     "lua_pushboolean"),
	"string": ("luaL_checkstring",  "lua_pushstring"),
	"fixed":  ("luaL_checkstring",  "lua_pushstring"),
	"handle": ("lua_touserdata",    "lua_pushlightuserdata"),
}

typedefs = {
	"nresults": "int32_t",
	"string":   "const char*",
	"handle":   "void*",
	"bool":     "bool_t",
	"uint":     "uint32_t",
	"long":     "long",
	"int":      "int32_t",
	"float":    "float",
	"fixed":    "fixedString_t",
}

def conv_name(name):
	for pat, repl in Axis:
		s2, n = pat.subn(repl, name, count=1)
		if n:
			return s2
	return name

def property_name(path):
	"""Transform a raw property path (e.g. 'Size.Axis[0].Requested') into
	a C identifier fragment (e.g. 'Width') using the Axis substitution rules."""
	name = conv_name(path)
	return "".join(s[:1].upper() + s[1:] for s in name.split('.') if s)

printers = {
	Kind.ATOMIC: "%s",
	Kind.ENUM: "enum %s",
	Kind.STRUCT: "struct %s",
	Kind.COMPONENT: "struct %s",
	Kind.RESOURCE: "struct %s",
	Kind.FIXED: "%sString_t",
	Kind.UNKNOWN: "%s_t",
}

class Model:
	def __init__(self, xml_file, include_file = None):
		tree = ET.parse(xml_file)
		self.root = tree.getroot()
		self.source = include_file
		self.requires = [Model(os.path.join(os.path.dirname(xml_file), req.get('file')), req.get('file')) for req in tree.getroot().findall('require')]
		self.structs = {s.get('name'): Struct(s, self) for s in self.root.findall(".//struct[@name]")}
		self.enums = {e.get('name'): Enum(e, self) for e in self.root.findall(".//enums[@name]")}
		self.components = {c.get('name'): Component(c, self) for c in self.root.findall(".//class[@name]")}
		self.resources = {c.get('type'): Resource(c, self) for c in self.root.findall(".//resource[@type]")}

	def _has_in(self, key, attr_name, visited=None):
		if visited is None: 
			visited = set()
		if id(self) in visited: 
			return None
		visited.add(id(self))
		if key in getattr(self, attr_name):
			return getattr(self, attr_name)[key]
		for req in self.requires:
			result = req._has_in(key, attr_name, visited)
			if result: 
				return result
		return None

	def getModuleName(self): return self.root.get('name')
	def getStruct(self, name): return self.structs.get(name)
	def getEnum(self, name): return self.enums.get(name)
	def getComponent(self, name): return self.components.get(name)
	def getResource(self, resource_type): return self.resources.get(resource_type)
	def getStructs(self): return self.structs
	def getEnums(self): return self.enums
	def getComponents(self): return self.components
	def getResources(self): return self.resources
	def hasClasses(self): return bool(self.root.find('class') is not None)
	def getElements(self):
		"""Yield all direct child elements of the module root in document order."""
		yield from self.root
	def getKind(self, _type):
		if _type == "fixed": return Kind.FIXED, None
		if _type in atomic_types: return Kind.ATOMIC, None
		if self._has_in(_type, "enums"): return Kind.ENUM, self._has_in(_type, "enums")
		if self._has_in(_type, "structs"): return Kind.STRUCT, self._has_in(_type, "structs")
		if self._has_in(_type, "components"): return Kind.COMPONENT, self._has_in(_type, "components")
		if self._has_in(_type, "resources"): return Kind.RESOURCE, self._has_in(_type, "resources")
		return Kind.UNKNOWN, None
	def getRequires(self): 
		for r in self.requires:
			yield (r.getModuleName(), r)

class Base:
	def __init__(self, element: ET.Element, model: Model):
		self._element = element
		self._model = model
		for k, v in element.attrib.items():
			setattr(self, k, v)

	def getName(self): return self._element.get('name')
	def getAttribute(self, key): return self._element.get(key)
	def getAttributes(self): return self._element.attrib
	def getSummary(self): return self._element.findtext('summary') or ''

class Type(Base):
	def __init__(self, element: ET.Element, model: Model):
		super().__init__(element, model)
		self.kind, self.data = model.getKind(self.type)
		self.fixed_array = int(element.get("fixed-array")) if element.get("fixed-array") else None

	def __str__(self):
		t = self.type
		is_pointer = getattr(self, 'pointer', None) or getattr(self, 'array', None)
		is_const = getattr(self, 'const', None)
		if t in typedefs:
			base = typedefs[t]
			return base + "*" if is_pointer else base
		if self.kind == Kind.ENUM:
			return f'e{t}_t'
		if self.kind in (Kind.STRUCT, Kind.COMPONENT, Kind.RESOURCE):
			if is_pointer:
				return f'lpc{t}_t' if is_const else f'lp{t}_t'
			return f'c{t}_t' if is_const else f'{t}_t'
		if self.kind == Kind.FIXED:
			return "fixedString_t"
		return f'{t}_t'

	def getDoc(self): return self._element.text or ''

class Method(Base):
	def __init__(self, element: ET.Element, model: Model, owner: ET.Element = None):
		super().__init__(element, model)
		self.args = [(arg.get('name'), Type(arg, model)) for arg in self._element.findall('arg')]
		self.static = self._element.get('static')
		if owner is not None and not self.static:
			self.args = [("this", Type(ET.Element("arg", {
				"name": "this", 
				"type": owner.get('name'), 
				"pointer": "true"
			}), model))] + self.args
		returns = self._element.find('returns')
		self.return_type = Type(returns, model) if returns is not None else 'void'
		self.full_name = f"{owner.get('prefix', '')}{self.getName()}" if owner is not None else self.getName()

	def getArgs(self):
		for name, type_ in self.args:
			yield name, type_
	
	def getArgsTypes(self):
		for _, type_ in self.args:
			yield type_

class Struct(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)
		self.sealed = element.get('sealed') == "true"

	def getFields(self):
		for f in self._element.findall(".//field[@name]"):
			yield f.get('name'), Type(f, self._model)

	def getMethods(self):
		for m in self._element.findall(".//method[@name]"):
			yield m.get('name'), Method(m, self._model, self._element)

	def __iter__(self):
		return self.getFields()

	def __getitem__(self, key):
		for name, type_ in self.getFields():
			if name == key:
				return type_
		raise KeyError(key)

class Component(Struct):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)
		# Mirror the NumXxx injection that conv-module.py performs at runtime so
		# that getEnumProperties() and getProperties() include the count field.
		import xml.etree.ElementTree as _ET
		children = list(element)
		for i, p in reversed(list(enumerate(children))):
			if p.tag == "property" and p.get("array"):
				num_elem = _ET.Element('property', {'name': f"Num{p.get('name')}", 'type': 'int'})
				num_elem.text = f"Number of {p.get('name')}"
				element.insert(i + 1, num_elem)

	def getProperties(self):
		def walk(name_prefix, type_):
			yield name_prefix, type_
			if type_.kind == Kind.STRUCT and not type_.data.sealed:
				for k, v in type_.data.getFields():
					if v.fixed_array:
						for i in range(v.fixed_array):
							yield from walk(f"{name_prefix}.{k}[{i}]", v)
					else:
						yield from walk(f"{name_prefix}.{k}", v)
		for f in self._element.findall(".//property[@name]"):
			type_ = Type(f, self._model)
			yield from walk(f.get('name'), type_)

	def getEnumProperties(self):
		"""Return a list of C-identifier property names (with Axis transforms applied).

		Matches the enumeration order used by ``properties_writer.py`` so the
		generated ``enum XxxProperties`` and ``#define ID_Xxx_Yyy`` blocks are
		byte-for-byte identical to those produced by the Python plugin."""
		result = []
		if self._element.get('sealed'):
			return result
		for sh in self._element.findall("shorthand"):
			result.append(sh.get('name'))
		for raw_path, _ in self.getProperties():
			result.append(property_name(raw_path))
		return result

class Enum(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)

	def getValues(self):
		for e in self._element.findall(".//enum[@name]"):
			yield e.get('name'), e.text


class Resource(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)