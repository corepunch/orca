import os, re
import xml.etree.ElementTree as ET
from enum import Enum

# C typedefs used for argument/return type generation
_typedefs = {
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

	# ── extra accessors needed by export.php ──────────────────────────────

	def getNamespace(self):
		"""Return the module namespace (e.g. ``"orca"``)."""
		return self.root.get('namespace', '')

	def getOnLuaopen(self):
		"""Return the ``on-luaopen`` callback name, or ``None``."""
		return self.root.get('on-luaopen')

	def hasClasses(self):
		"""Return True when the module defines at least one ``<class>``."""
		return bool(self.root.find('class') is not None)

	def getFunctions(self):
		"""Yield ``(name, Function)`` pairs for each module-level function."""
		for fn in self.root.findall('function'):
			yield fn.get('name'), Function(fn, self)

	def getShutdowns(self):
		"""Yield ``Shutdown`` objects for each ``<shutdown>`` element."""
		for s in self.root.findall('shutdown'):
			yield Shutdown(s, self)

	def getInterfaces(self):
		"""Return a ``{name: Interface}`` dict for ``<interface>`` elements."""
		return {i.get('name'): Interface(i, self)
		        for i in self.root.findall('interface')}

	def hasShutdown(self):
		"""Return True when the module has a ``<shutdown>`` element."""
		return bool(self.root.find('shutdown') is not None)

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
		self.export = self.data.export if self.kind == Kind.STRUCT else self.type

	def __str__(self):
		base = printers.get(self.kind, "%s") % self.type
		if getattr(self, 'const', False): base += " const"
		if getattr(self, 'pointer', False): base += "*"
		return base
	
	def getImporter(self, index):
		if self.kind == Kind.ATOMIC or self.kind == Kind.FIXED:
			return f"{atomic_types[self.type][0]}(L, {index})"
		elif self.kind == Kind.ENUM:
			return f"luaL_checkoption(L, index, NULL, _{self.type})"
		elif self.kind in (Kind.STRUCT, Kind.COMPONENT, Kind.RESOURCE):
			return f"luaX_check{self.type}(L, {index})"
		else:
			return None

	def getExporter(self, variable):
		if self.kind == Kind.ATOMIC or self.kind == Kind.FIXED:
			return f"{atomic_types[self.type][1]}(L, {variable})"
		elif self.kind == Kind.ENUM:
			return f"lua_pushstring(L, {self.type}ToString({variable}))"
		elif self.kind in (Kind.STRUCT, Kind.COMPONENT, Kind.RESOURCE):
			return f"luaX_push{self.type}(L, {variable})"
		else:
			return None

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
		self.id = "0x%s"%fnv1a32(name)
		return self

class PropertyName:
	def __init__(self, classname, *args):
		self.classname = classname
		self.path = args
		
	def __str__(self): return self.format()
	def getPath(self): return '.'.join(self.path)
	def format(self):
		name = self.getPath()
		for pat, repl in Axis:
			s2, n = pat.subn(repl, name, count=1)
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

class Component(Struct):
	def __init__(self, element: ET.Element, model: Model): 
		super().__init__(element, model)

	def getProperties(self):
		def walk(type_, *args):
			yield PropertyName(*args), type_
			if type_.kind == Kind.STRUCT and not type_.data.sealed:
				for k, v in type_.data.getFields():
					if v.fixed_array:
						for i in range(v.fixed_array):
							yield from walk(v, *args, f"{k}[{i}]")
					else:
						yield from walk(v, *args, k)

		for f in self._element.findall(".//property[@name]"):
			type_ = Type(f, self._model)
			yield from walk(type_, self.name, f.get('name'))

	# ── extra accessors needed by export.php ──────────────────────────────

	def getHandles(self):
		"""Yield event names handled by this component."""
		for h in self._element.findall('handles'):
			yield h.get('event')

	def getParents(self):
		"""Return a list of parent class names (parent + concept attributes)."""
		parents = []
		if self._element.get('parent'):
			parents.append(self._element.get('parent'))
		if self._element.get('concept'):
			parents.append(self._element.get('concept'))
		return parents

	def getDefaultName(self):
		"""Return the ``default`` attribute, falling back to the class name."""
		return self._element.get('default', self.name)

	def getContentType(self):
		"""Return the content-type string used in ClassDesc."""
		cname = self._element.get('name')
		content = self._element.get('content', self._element.get('parent', cname))
		if cname.endswith('Library'):
			content = cname.replace('Library', '')
		return content

	def getXmlns(self):
		"""Return the ``<xmlns>`` text value, or an empty string."""
		return self._element.findtext('xmlns') or ''

	def hasDefaultProperties(self):
		"""Return True when at least one property has a ``default`` attribute."""
		return bool(self._element.findall("property[@default]"))

	def getDefaultPropertyValues(self):
		"""Yield :class:`DefaultProperty` objects for properties with ``default`` attributes."""
		for prop in self._element.findall("property[@default]"):
			yield DefaultProperty(prop.get('name'), prop.get('type'), prop.get('default'))

	def getPropertyDecls(self):
		"""Yield :class:`PropertyDecl` objects for the PropertyType C array."""
		def _walk_component(component_elem, path, is_array_prop):
			"""Walk component properties recursively, matching enum_component_properties."""
			# Shorthands at component level
			for sh in component_elem.findall('shorthand'):
				phys = sh.get('physical')
				ptype = sh.get('type', '')
				yield _make_decl(component_elem, sh, phys, is_array_prop)

			tag = 'property' if component_elem.tag == 'class' else 'field'
			for prop in component_elem.findall(tag):
				pname = prop.get('name')
				fixed = prop.get('fixed-array')
				if fixed:
					for n in range(int(fixed)):
						yield from _walk_property(component_elem, prop, f'{pname}[{n}]', False)
				else:
					yield from _walk_property(component_elem, prop, pname, bool(prop.get('array')))

		def _walk_property(component_elem, prop_elem, path, is_array):
			if not prop_elem.get('exclude-self'):
				yield _make_decl(component_elem, prop_elem, path, is_array)
			struct_obj = self._model._has_in(prop_elem.get('type', ''), 'structs')
			if struct_obj is not None:
				yield from _walk_struct(component_elem, struct_obj, path + '.')

		def _walk_struct(component_elem, struct_obj, prefix):
			# Shorthands on struct
			for sh in struct_obj._element.findall('shorthand'):
				phys = sh.get('physical')
				yield _make_decl(component_elem, sh, phys, False)
			if struct_obj.sealed:
				return
			for field in struct_obj._element.findall('field'):
				fname = field.get('name')
				fixed = field.get('fixed-array')
				if fixed:
					for n in range(int(fixed)):
						yield from _walk_property(component_elem, field, f'{prefix}{fname}[{n}]', False)
				else:
					yield from _walk_property(component_elem, field, prefix + fname, False)

		def _make_decl(component_elem, prop_elem, path, is_array):
			cname = component_elem.get('name')
			# Compute the short formatted name using the same Axis substitutions
			pname = _property_name(path)
			ptype = prop_elem.get('type', '')

			struct_obj    = self._model._has_in(ptype, 'structs')
			enum_obj      = self._model._has_in(ptype, 'enums')
			# Also check by short name for inline enums
			if enum_obj is None:
				enum_obj  = self._model._has_in(pname, 'enums')
			component_obj = self._model._has_in(ptype, 'components')

			if struct_obj is not None:
				exp = struct_obj.export
				typedata = f'kDataTypeStruct, .TypeString="{exp}"'
			elif enum_obj is not None:
				values = [e.get('name') for e in enum_obj._element.findall('enum')]
				ptype = enum_obj._element.get('name')
				typedata = f'kDataTypeEnum, .TypeString="{",".join(values)}"'
			elif component_obj is not None:
				typedata = f'kDataTypeObject, .TypeString="{ptype}"'
			else:
				typedata = f'kDataType{ptype[:1].upper() + ptype[1:]}'

			return PropertyDecl(
				component_name=cname,
				short_name=pname,
				field_path=path,
				typedata=typedata,
				is_array=is_array,
			)

		if not self._element.get('sealed'):
			yield from _walk_component(self._element, '', False)

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

	def isLua(self):
		"""Return True when the resource exposes a Lua binding (no ``no-lua`` attr)."""
		return not self._element.get('no-lua')

	def getTypeName(self):
		"""Return the resource type name."""
		return self._element.get('type', '')


def formatDefault(ptype, pdefault, model):
	"""Format a property default value as a C initializer expression."""
	kind, _ = model.getKind(ptype)
	if kind == Kind.ENUM:
		if not pdefault.lstrip('-').isdigit():
			return f'k{ptype}{pdefault}'
		return pdefault
	if kind == Kind.STRUCT:
		return f'(struct {ptype}) {{ {", ".join(pdefault.split())} }}'
	if ptype == 'bool':
		return pdefault.upper()
	if kind == Kind.FIXED or ptype == 'fixed':
		return f'"{pdefault}"'
	return pdefault


# ---------------------------------------------------------------------------
# Helper: property-name formatter  (mirrors utils.property_name)
# ---------------------------------------------------------------------------

def _property_name(path):
	"""Convert a dotted property path to a CamelCase C identifier."""
	def _conv(name):
		for pat, repl in Axis:
			s2, n = pat.subn(repl, name, count=1)
			if n:
				return s2
		return name
	name = _conv(path)
	return ''.join(s[0].upper() + s[1:] for s in name.split('.'))


# ---------------------------------------------------------------------------
# Function argument / return type helpers
# ---------------------------------------------------------------------------

def _c_arg_type(arg_elem, model):
	"""Return the C type string for a function argument element."""
	t = arg_elem.get('type', '')
	is_pointer = bool(arg_elem.get('pointer') or arg_elem.get('array'))
	is_const   = bool(arg_elem.get('const'))
	if t in _typedefs:
		return f"{_typedefs[t]}*" if is_pointer else _typedefs[t]
	if model._has_in(t, 'enums'):
		return f"e{t}_t"
	if is_pointer:
		return f"lpc{t}_t" if is_const else f"lp{t}_t"
	return f"c{t}_t" if is_const else f"{t}_t"


class FunctionArg(Base):
	"""Wraps a ``<arg>`` element inside a ``<function>`` or ``<method>``."""

	def __init__(self, element: ET.Element, model: Model):
		super().__init__(element, model)

	def getCType(self):
		"""Return the C type declaration string (e.g. ``"const char*"``)."""
		return _c_arg_type(self._element, self._model)

	def getCheckDecl(self, idx):
		"""Return the C statement that checks and declares this arg from Lua.

		Example: ``"const char* path = luaL_checkstring(L, 1)"``
		"""
		t      = self._element.get('type', '')
		name   = self._element.get('name', '')
		ctype  = self.getCType()
		target = f'{ctype} {name}'
		if t == 'fixed':
			return f'strncpy({name}, luaL_checkstring(L, {idx}), sizeof({name}))'
		if t in atomic_types:
			check, _ = atomic_types[t]
			return f'{target} = {check}(L, {idx})'
		return f'{target} = luaX_check{t}(L, {idx})'


class ReturnType(Base):
	"""Wraps a ``<returns>`` element."""

	def __init__(self, element: ET.Element, model: Model):
		super().__init__(element, model)

	def isNresults(self):
		"""Return True when the return type is ``nresults`` (variable int)."""
		return self._element.get('type') == 'nresults'

	def getCType(self):
		"""Return the C type string for the return value."""
		return _c_arg_type(self._element, self._model)

	def __str__(self):
		return self.getCType()

	def getPushExpr(self, var):
		"""Return the C expression that pushes *var* onto the Lua stack."""
		t          = self._element.get('type', '')
		is_pointer = bool(self._element.get('pointer') or self._element.get('array'))
		if t in atomic_types:
			_, push = atomic_types[t]
			return f'{push}(L, {var})'
		if is_pointer or self._model._has_in(t, 'enums'):
			return f'luaX_push{t}(L, {var})'
		return f'luaX_push{t}(L, &{var})'


# ---------------------------------------------------------------------------
# Module-level function, shutdown, and interface wrappers
# ---------------------------------------------------------------------------

def _camel_case(name):
	"""Return *name* with its first letter lowercased (PHP camelCase helper)."""
	return name[0].lower() + name[1:] if name else name


class Function(Base):
	"""Wraps a ``<function>`` element at the module root level."""

	def __init__(self, element: ET.Element, model: Model):
		super().__init__(element, model)
		self._module_name = model.getModuleName()

	def isPrivate(self):
		"""Return True when the function is marked ``private``."""
		return self._element.get('private') is not None

	def isLua(self):
		"""Return True when the function receives the Lua state directly."""
		return self._element.get('lua') is not None

	def getExportName(self):
		"""Return the C export wrapper name (e.g. ``f_geometry_vec2Scale``)."""
		return f"f_{self._module_name}_{_camel_case(self._element.get('name', ''))}"

	def getLuaName(self):
		"""Return the Lua-visible name (camelCase of the XML name)."""
		return _camel_case(self._element.get('name', ''))

	def getArgs(self):
		"""Yield :class:`FunctionArg` objects for each ``<arg>`` child."""
		for arg in self._element.findall('arg'):
			yield FunctionArg(arg, self._model)

	def getReturnType(self):
		"""Return a :class:`ReturnType` for ``<returns>``, or ``None``."""
		ret = self._element.find('returns')
		return ReturnType(ret, self._model) if ret is not None else None

	def getCallExpr(self):
		"""Return the C call expression for this function."""
		args = []
		if self.isLua():
			args.append('L')
		for arg in self._element.findall('arg'):
			args.append(arg.get('name', ''))
		fn_name = f"{self._module_name}_{self._element.get('name', '')}"
		return f"{fn_name}({', '.join(args)})"


class Shutdown(Base):
	"""Wraps a ``<shutdown>`` element."""

	def __init__(self, element: ET.Element, model: Model):
		super().__init__(element, model)

	def getFunctionName(self):
		"""Return the name of the shutdown callback."""
		return self._element.get('name', '')


class Interface(Base):
	"""Wraps an ``<interface>`` element (lightweight struct-like binding)."""

	def __init__(self, element: ET.Element, model: Model):
		super().__init__(element, model)
		self.export = element.get('export') or element.get('name')

	def getMethods(self):
		"""Yield ``(name, Method)`` pairs for each ``<method>`` child."""
		for m in self._element.findall('method'):
			yield m.get('name'), Method(m, self._model, self._element)

	def hasInit(self):
		"""Return True when the interface defines an ``<init>`` element."""
		return self._element.find('init') is not None

	def getExportName(self):
		"""Return the export/Lua name for this interface."""
		return self._element.get('export') or self._element.get('name')

	def hasNoCheck(self):
		return bool(self._element.get('no-check'))


# ---------------------------------------------------------------------------
# Property declaration helper  (used by Component.getPropertyDecls)
# ---------------------------------------------------------------------------

class DefaultProperty:
	"""Simple value-object for a property with a default value."""
	def __init__(self, name, type_str, default):
		self.name     = name
		self.type_str = type_str
		self.default  = default


class PropertyDecl:
	"""Represents one row in the ``PropertyType`` C array for a component."""

	def __init__(self, component_name, short_name, field_path, typedata, is_array):
		self.component_name = component_name
		self.short_name     = short_name    # e.g. "PositionX"
		self.field_path     = field_path    # e.g. "Position.X"
		self.typedata       = typedata      # e.g. "kDataTypeFloat"
		self.is_array       = is_array

	def getDecl(self):
		"""Return the DECL / ARRAY_DECL macro invocation line."""
		hval  = fnv1a32(self.short_name)   # already a hex string without 0x
		macro = 'ARRAY_DECL' if self.is_array else 'DECL'
		return (
			f'\t{macro}(0x{hval}, {self.component_name}, {self.short_name}, '
			f'{self.field_path}, {self.typedata}), '
			f'// {self.component_name}.{self.short_name}'
		)