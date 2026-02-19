"""DTD schema writer plugin."""

from . import Plugin
from .state import g_enums, g_components
from .utils import enum_component_properties, property_name, hash


class DTDWriter(Plugin):
	"""Generates a DTD schema file from module XML."""

	def __init__(self):
		self.file = open("schemas/orca.dtd", "w")

	def write(self, text):
		self.file.write(text)

	def finalize(self, append_path="schemas/append.dtd"):
		with open(append_path) as f:
			self.file.write(f.read())
		self.file.close()

	def on_enums(self, _, enums):
		values = [e.get('name') for e in enums.findall('enum')]
		self.write(f"<!ENTITY % {enums.get('name')} \"{'|'.join(values)}\">\n")

	def _collect_struct_fields(self, struct, prefix, attribs):
		from .state import g_structs
		if struct.get('sealed'):
			return
		for field in struct.findall('field'):
			fname = field.get('name')
			if field.get('array'):
				for idx in range(int(field.get('array'))):
					self._collect_property(field, f"{prefix}{fname}[{idx}]", attribs)
			else:
				self._collect_property(field, f"{prefix}{fname}", attribs)

	def _collect_property(self, prop, path, attribs):
		from .state import g_structs
		prop_type = prop.get('type')
		struct = g_structs.get(prop_type)
		if not prop.get('exclude-self'):
			sname = property_name(path)
			enum = g_enums.get(prop_type) or (g_enums.get(sname) if struct is None else None)
			ptype = enum.get('name') if enum is not None else prop_type
			attribs.append((sname, ptype))
		if struct is not None:
			self._collect_struct_fields(struct, f"{path}.", attribs)

	def _iter_component_attribs(self, component):
		attribs = []
		for prop in component.findall("property"):
			pname = prop.get('name')
			if prop.get('array'):
				for idx in range(int(prop.get('array'))):
					self._collect_property(prop, f"{pname}[{idx}]", attribs)
			else:
				self._collect_property(prop, pname, attribs)
		return attribs

	def _is_component(self, comp, suffix):
		if comp.get('name') == "Viewport3D":
			return False
		if comp and suffix in comp.get('name'):
			return True
		if comp.get('parent') is None:
			return False
		return self._is_component(g_components.get(comp.get('parent')), suffix)

	def on_component(self, _, component):
		cname = component.get('name')
		self.write(f"<!ENTITY % {cname}Attribs \"\n")
		if component.get('parent'):
			self.write(f"\t%{component.get('parent')}Attribs;\n")
			if component.get('concept'):
				self.write(f"\t%{component.get('concept')}Attribs;\n")
		else:
			self.write("\tName CDATA #REQUIRED\n")
			self.write("\tStyle CDATA #IMPLIED\n")
			self.write("\tIsDisabled (true|false) #IMPLIED\n")
			self.write("\tStateManager CDATA #IMPLIED\n")

		attribs = self._iter_component_attribs(component)
		for sname, ptype in attribs:
			if ptype in g_enums:
				self.write(f"\t{sname} (%{ptype};) #IMPLIED\n")
			elif ptype == 'bool':
				self.write(f"\t{sname} (true|false) #IMPLIED\n")
			else:
				self.write(f"\t{sname} CDATA #IMPLIED\n")
		self.write("\">\n")

		for sname, _ in attribs:
			self.write(f"<!ELEMENT {cname}.{sname} (#PCDATA | Binding)*>\n")
			self.write(
				"<!ATTLIST "
				f"{cname}.{sname} Mode (expression|oneway) #IMPLIED "
				"Attribute CDATA #IMPLIED Enabled (true|false) #IMPLIED>\n"
			)

		element_attribs = [f"{cname}.{sname}" for sname, _ in attribs]
		if component.get("parent"):
			element_attribs.append(f"%{component.get('parent')}Elements;")
		if component.get("concept"):
			element_attribs.append(f"%{component.get('concept')}Elements;")
		self.write(f"<!ENTITY % {cname}Elements \"{'|'.join(element_attribs)}\">\n")

		self.write(f"<!ELEMENT {cname} ")
		elm = f"%{cname}Elements;"
		if self._is_component(component, '2D'):
			arr = [key for key, value in g_components.items() if self._is_component(value, "2D")]
			arr += ["Viewport3D", "Resource", "StyleSheet", "LayerPrefabPlaceholder", elm]
			self.write(f"({'|'.join(arr)})*>\n")
		elif self._is_component(component, '3D'):
			arr = [key for key, value in g_components.items() if self._is_component(value, "3D")]
			arr += ["Resource", "StyleSheet", "ObjectPrefabPlaceholder", elm]
			self.write(f"({'|'.join(arr)})*>\n")
		elif component.get('name') == "Viewport3D":
			arr = [key for key, _ in g_components.items()]
			arr += ["Resource", "StyleSheet", "ObjectPrefabPlaceholder", elm]
			self.write(f"({'|'.join(arr)})*>\n")
		else:
			self.write("EMPTY>\n")
		self.write(f"<!ATTLIST {cname} %{cname}Attribs;>\n")
