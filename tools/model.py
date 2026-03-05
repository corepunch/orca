import os
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

printers = {
	Kind.ATOMIC: "%s",
	Kind.ENUM: "enum %s",
	Kind.STRUCT: "struct %s",
	Kind.COMPONENT: "struct %s",
	Kind.RESOURCE: "struct %s",
	Kind.FIXED: "%sString_t",
	Kind.UNKNOWN: "%s",
}

class Model:
	def __init__(self, xml_file):
		tree = ET.parse(xml_file)
		self.root = tree.getroot()
		self.requires = [Model(os.path.join(os.path.dirname(xml_file), req.get('file'))) for req in tree.getroot().findall('require')]
		self.structs = {s.get('name'): Struct(s, self) for s in self.root.findall(".//struct[@name]")}
		self.enums = {e.get('name'): Enum(e, self) for e in self.root.findall(".//enums[@name]")}
		self.components = {c.get('name'): Component(c, self) for c in self.root.findall(".//class[@name]")}
		self.resources = {c.get('type'): Resource(c, self) for c in self.root.findall(".//resource[@type]")}

	def getModuleName(self): return self.root.get('name')
	def getStruct(self, name): return self.structs.get(name)
	def getEnum(self, name): return self.enums.get(name)
	def getComponent(self, name): return self.components.get(name)
	def getResource(self, resource_type): return self.resources.get(resource_type)
	def getStructs(self): return self.structs
	def getEnums(self): return self.enums
	def getComponents(self): return self.components
	def getResources(self): return self.resources

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
		self.kind = self.getKind(model)

	def __str__(self):
		base = printers.get(self.kind, "%s") % self.type
		if getattr(self, 'const', False): base += " const"
		if getattr(self, 'pointer', False): base += "*"
		return base

	def getKind(self, model):
		if self.type == "fixed":           return Kind.FIXED
		if self.type in atomic_types:      return Kind.ATOMIC
		if self.type in model.enums:       return Kind.ENUM
		if self.type in model.structs:     return Kind.STRUCT
		if self.type in model.components:  return Kind.COMPONENT
		if self.type in model.resources:   return Kind.RESOURCE
		return Kind.UNKNOWN

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
		for f in self._element.findall(".//property[@name]"):
			yield f.get('name'), Type(f, self._model)

class Enum(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)

	def getValues(self):
		for e in self._element.findall(".//enum[@name]"):
			yield e.get('name'), e.text


class Resource(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)