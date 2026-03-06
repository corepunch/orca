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

def conv_name(name):
	for pat, repl in Axis:
		s2, n = pat.subn(repl, name, count=1)
		if n:
			return s2
	return name

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

class Type(Base):
	def __init__(self, element: ET.Element, model: Model):
		super().__init__(element, model)
		self.kind, self.data = model.getKind(self.type)
		self.fixed_array = int(element.get("fixed-array")) if element.get("fixed-array") else None

	def __str__(self):
		base = printers.get(self.kind, "%s") % self.type
		if getattr(self, 'const', False): base += " const"
		if getattr(self, 'pointer', False): base += "*"
		return base

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

class Enum(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)

	def getValues(self):
		for e in self._element.findall(".//enum[@name]"):
			yield e.get('name'), e.text


class Resource(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)