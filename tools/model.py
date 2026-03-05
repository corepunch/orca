import xml.etree.ElementTree as ET

class Base:
	def __init__(self, element: ET.Element):
		self._element = element
		for k, v in element.attrib.items():
			setattr(self, k, v)

	def getName(self): return self._element.get('name')
	def getAttribute(self, key): return self._element.get(key)
	def getAttributes(self): return self._element.attrib

class Struct(Base):
	def __init__(self, element: ET.Element): super().__init__(element)

	def getFields(self):
		for f in self._element.findall(".//field[@name]"):
			yield f.get('name'), f.get('type')

	def __iter__(self):
		return self.getFields()

	def __getitem__(self, key):
		for name, type_ in self.getFields():
			if name == key:
				return type_
		raise KeyError(key)
		
class Model:
	def __init__(self, xml_file):
		tree = ET.parse(xml_file)
		self.root = tree.getroot()
		self.structs = {s.get('name'): Struct(s) for s in self.root.findall(".//struct[@name]")}
		self.enums = {e.get('name'): e for e in self.root.findall(".//enums[@name]")}
		self.components = {c.get('name'): c for c in self.root.findall(".//class[@name]")}
		self.resources = {c.get('type'): c for c in self.root.findall(".//resource[@type]")}

	def getDeclType(self, type_name):
		if type_name in self.structs:
			return f"struct {type_name}"
		if type_name in self.enums:
			return f"enum {type_name}"
		return type_name
	
	def getStruct(self, name):
		return self.structs.get(name)
	
	def getEnum(self, name):
		return self.enums.get(name)
	
	def getComponent(self, name):
		return self.components.get(name)
	
	def getResource(self, resource_type):
		return self.resources.get(resource_type)
	
	def getStructs(self):
		return self.structs
	
	def getEnums(self):
		return self.enums
	
	def getComponents(self):
		return self.components
	
	def getResources(self):
		return self.resources
