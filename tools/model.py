import os, re
import xml.etree.ElementTree as ET
import config
from enum import Enum

class Model:
	def __init__(self, xml_file, include_file = None):
		tree = ET.parse(xml_file)
		self.root = tree.getroot()
		self.source = include_file or xml_file
		self.requires = [Model(os.path.join(os.path.dirname(xml_file), req.get('file')), req.get('file')) for req in tree.getroot().findall('require')]
		self.structs = {s.get('name'): Struct(s, self) for s in self.root.findall(".//struct[@name]")}
		self.enums = {e.get('name'): Enum(e, self) for e in self.root.findall(".//enums[@name]")}
		self.components = {c.get('name'): Component(c, self) for c in self.root.findall(".//class[@name]")}
		self.resources = {c.get('type'): Resource(c, self) for c in self.root.findall(".//resource[@type]")}

	def _has_in(self, key, attr_name):
		if key in getattr(self, attr_name):
			return getattr(self, attr_name)[key]
		for req in self.requires:
			result = req._has_in(key, attr_name)
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
		if self._has_in(_type, "enums"): return "enum", self._has_in(_type, "enums")
		if self._has_in(_type, "structs"): return "struct", self._has_in(_type, "structs")
		if self._has_in(_type, "components"): return "component", self._has_in(_type, "components")
		if self._has_in(_type, "resources"): return "resource", self._has_in(_type, "resources")
		return _type, None
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
		self.export = self.data.export if self.kind == "struct" else self.type
		self.default = element.get("default")

	def __str__(self):
		base = {
			"enum": "enum %s",
			"struct": "struct %s",
			"component": "struct %s",
			"resource": "struct %s",
			"fixed": "%sString_t",
		}.get(self.kind, "%s") % self.type
		if getattr(self, 'const', False): base += " const"
		if getattr(self, 'pointer', False): base += "*"
		return base

	def get(self, name, arg = None, addr = None):
		return config.TypeInfos[self.kind][name].format(type = self.type, arg = arg, addr = addr)
	
def fnv1a32(str):
	hval = 0x811C9DC5
	prime = 0x01000193
	uint32_max = 2 ** 32
	for s in str:
		hval = hval ^ ord(s)
		hval = (hval * prime) % uint32_max
	return "%08x" % hval	

class FieldName(str):
	def __new__(cls, name):
		self = super().__new__(cls, name)
		self.id = "0x%s" % fnv1a32(name)
		return self

class PropertyName:
	def __init__(self, classname, *args):
		self.classname = classname
		self.path = args
		self.addr = '.'.join(args)
		self.id = "0x%s" % fnv1a32(''.join(args))
		
	def __str__(self): return self.format()
	def getPath(self): return '.'.join(self.path)
	def format(self):
		name = self.getPath()
		for pat, repl in config.Axis:
			s2, n = re.compile(pat).subn(repl, name, count=1)
			if n:
				return s2.replace('.', '')
		return name.replace('.', '')

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

class ParserType(str):
	def __new__(cls, name, addr, type_):
		self = super().__new__(cls, name)
		self.addr = addr
		self.pointer = f"&{name}" if type_.get("format") != "%s" else name
		return self

class Struct(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)
		self.sealed = element.get('sealed') == "true"
		self.export = element.get('export') or self.name

	def getFields(self):
		for f in self._element.findall(".//field[@name]"):
			yield FieldName(f.get('name')), Type(f, self._model)

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

	def getParsers(self):
		def poll():
			for name, field in self.getFields():
				if field.fixed_array:
					for i in range(field.fixed_array):
						if field.kind == "struct":
							for sub_name, sub_type in field.data.getFields():
								yield ParserType(f"{self.name}_{name}{i}_{sub_name}", f"{name}[{i}].{sub_name}", sub_type), sub_type
						else:
							yield ParserType(f"{self.name}_{name}{i}", f"{name}[{i}]", field), field
				else:
					yield ParserType(name, name, field), field
		return dict(poll())

	def getConstructors(self):
		if self._element.get("constructor"):
			for n in self._element.get("constructor").split(','):
				yield int(n)

class Component(Struct):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)

	def getProperties(self):
		def func():
			def walk(type_, *args):
				yield PropertyName(*args), type_
				if type_.kind == "struct" and not type_.data.sealed:
					for k, v in type_.data.getFields():
						if v.fixed_array:
							for i in range(v.fixed_array):
								yield from walk(v, *args, f"{k}[{i}]")
						else:
							yield from walk(v, *args, k)

			for f in self._element.findall(".//property[@name]"):
				type_ = Type(f, self._model)
				yield from walk(type_, self.name, f.get('name'))
		return dict(func())

	def getEventHandlers(self):
		for node in self._element.findall("handles"):
			yield node.get("event")

class Enum(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)

	def getValues(self):
		for e in self._element.findall(".//enum[@name]"):
			yield e.get('name'), e.text

	def getValuesNames(self):
		for e in self._element.findall(".//enum[@name]"):
			yield e.get('name')

class Resource(Base):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)