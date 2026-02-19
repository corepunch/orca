"""Component properties header writer plugin."""

import os

from . import Plugin
from .utils import enum_component_properties, hash, property_name


class PropertiesWriter(Plugin):
	"""Generates a C properties header (_properties.h) from module XML."""

	def __init__(self):
		self.file = None

	def open(self, xml, root):
		base, _ = os.path.splitext(xml)
		self.file = open(base + "_properties.h", "w")

	def w(self, text):
		self.file.write(text)
		self.file.write("\n")

	def write_property_enum(self, property, component, path):
		name = property.get('name') if property.tag == 'shorthand' else property_name(path)
		self.w(f"\tk{component.get('name')}{name},")

	def write_property_id(self, property, component, path):
		name = property.get('name') if property.tag == 'shorthand' else property_name(path)
		cname = component.get('name')
		self.w(f"#define ID_{cname}_{name} {hash(cname + '.' + name)}")

	def on_component(self, root, component):
		cname = component.get('name')
		self.w(f"#define ID_{cname} " + hash(cname))
		self.w(
			f"#define Get{cname}(_P)"
			f"((struct {cname}*)((_P)?OBJ_GetComponent(_P,ID_{cname}):NULL))"
		)
		self.w(
			f"#define {cname}_GetProperty(_P,_N)"
			f"OBJ_GetPropertyAtIndex(_P,ID_{cname},sizeof(struct {cname}),_N)"
		)
		self.w(f"enum {cname}Properties {{")
		enum_component_properties(component, self.write_property_enum)
		self.w(f"\tk{cname}NumProperties\n}};\n")
		enum_component_properties(component, self.write_property_id)
