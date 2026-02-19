"""
ORCA Module Converter - Plugin Architecture

This tool converts XML module definitions into generated code for C, Lua bindings,
and HTML documentation.

Plugin Architecture:
-------------------
The tool uses a plugin-based architecture for different output formats.
Plugins are located in the plugins/ directory:

1. HeaderPlugin (plugins/header_plugin.py) - Generates C header files (.h)
2. ExportPlugin (plugins/export_plugin.py) - Generates Lua binding code (_export.c)
3. PropsPlugin (plugins/props_plugin.py) - Generates component property metadata (_properties.h)
4. HtmlPlugin (plugins/html_plugin.py) - Generates HTML documentation

Usage:
------
Basic usage:
    python3 conv-module.py <module.xml> [module2.xml ...]

Custom Plugin Example:
----------------------
To create a custom output plugin, create a new file in plugins/:

    # plugins/my_plugin.py
    from . import OutputPlugin
    
    class MyCustomPlugin(OutputPlugin):
        def __init__(self, filename):
            super().__init__(filename, ".mycustom")
    
Then in conv-module.py:
    from plugins.my_plugin import MyCustomPlugin
    PluginRegistry.register('mycustom', MyCustomPlugin)
    
    # Enable/disable plugins
    PluginRegistry.disable_plugin('html')  # Disable HTML generation
    PluginRegistry.enable_plugin('mycustom')  # Enable custom plugin

See PLUGIN_ARCHITECTURE.md for complete documentation.
"""

import sys, re, io, os
import xml.etree.ElementTree as ET

# Import plugin system
from plugins import BasePlugin, OutputPlugin, PluginRegistry
from plugins.header_plugin import HeaderPlugin
from plugins.export_plugin import ExportPlugin
from plugins.props_plugin import PropsPlugin
from plugins.html_plugin import HtmlPlugin, DocumentationGenerator, tokenize, rules, wrap_backticks
from plugins.common import (
	w, wf, lpc, lp, _c, _t, _e, lpobject_t,
	camel_case, fnv1a32, hash,
	atomic_types, typedefs, Axis
)

# Register built-in plugins
PluginRegistry.register('header', HeaderPlugin)
PluginRegistry.register('export', ExportPlugin)
PluginRegistry.register('props', PropsPlugin)
PluginRegistry.register('html', HtmlPlugin)

# Create global documentation generator instance
doc = DocumentationGenerator()

g_structs = {}
g_enums = {}
g_components = {}
g_resources = {}
g_attribs = []
g_article = None
g_html = ET.Element('html', attrib={'xmlns':'http://www.w3.org/2001/XMLSchema' })
g_head = ET.SubElement(g_html, 'head')
ET.SubElement(g_head, 'link', { "rel":"stylesheet", "href":"doc.css" })
ET.SubElement(g_head, 'link', { "rel":"stylesheet", "href":"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css" })
ET.SubElement(g_head, 'meta', {'charset':"UTF-8"})
g_body = ET.SubElement(g_html, 'body')
g_wrapper = ET.SubElement(g_body, 'div', {'class': 'adjustable-sidebar-width full-width-container topic-wrapper'})
g_sidebar = ET.SubElement(g_wrapper, 'div', {'class': 'sidebar'})
g_content = ET.SubElement(g_wrapper, 'main', {'class': 'content'})
g_dtd = open("schemas/orca.dtd", "w")
g_output = "../docs/index.html"

class ParserState:
	def __init__(self, filename):
		# Initialize plugins using the registry
		self.plugins = {}
		self.filename = filename
		
		# Initialize file-based plugins (header, export, props)
		for plugin_name in ['header', 'export', 'props']:
			self._initialize_file_plugin(plugin_name, filename)
		
		# Initialize html plugin (no file needed)
		self._initialize_plugin('html')
		
		# Set up cross-plugin references
		if self.export_plugin and self.header_plugin:
			self.export_plugin.header_plugin = self.header_plugin
	
	def set_globals(self, structs, enums, components, resources):
		"""Set global state on all plugins"""
		if self.header_plugin:
			self.header_plugin.set_globals(structs, enums)
		if self.export_plugin:
			self.export_plugin.set_globals(structs, enums, self.header_plugin, components, resources)
		# Set globals on the doc generator
		doc.set_globals(structs, enums, components)
	
	def _initialize_file_plugin(self, name, filename):
		"""Helper method to initialize file-based plugins"""
		plugin = PluginRegistry.create_plugin(name, filename)
		if plugin:
			file_handle = plugin.open()
			setattr(self, name, file_handle)
			self.plugins[name] = plugin
			setattr(self, f'{name}_plugin', plugin)  # Keep for backward compatibility
		else:
			setattr(self, name, None)
			setattr(self, f'{name}_plugin', None)
	
	def _initialize_plugin(self, name):
		"""Helper method to initialize non-file plugins"""
		plugin = PluginRegistry.create_plugin(name)
		if plugin:
			self.plugins[name] = plugin
			setattr(self, f'{name}_plugin', plugin)  # Keep for backward compatibility
		else:
			setattr(self, f'{name}_plugin', None)
	
	def close_all(self):
		"""Close all plugin files"""
		for plugin in self.plugins.values():
			if hasattr(plugin, 'close'):
				plugin.close()
	
	def get_plugin(self, name):
		"""Get a specific plugin by name"""
		return self.plugins.get(name)
	
def throw_no_parser(root, node, _):
	print("no such parser: ", node.tag)
	# exit(0)

def header_get_method_this(node, method):
	if method.get('const'):
		return f"{lpc(node.get('name'))} self"
	else:
		return f"{lp(node.get('name'))} self"

def header_get_method_name(node, method):
	return f"{node.get('prefix', node.get('name') + '_')}{method.get('name')}"

def header_get_arg_type(arg):
	t = arg.get('type')
	if t in typedefs:
		return f"{typedefs[t]}*" if arg.get('pointer') else typedefs[t]
	if t in g_enums:
		return _e(t)
	if arg.get('pointer'):
		return lpc(t) if arg.get('const') else lp(t)
	else:
		return _c(t) if arg.get('const') else _t(t)

def header_get_method_arg(arg):
	return f"{header_get_arg_type(arg)} {arg.get('name')}"

def header_write_method(node, method, file):
	args = []
	if method.get("lua"):
		args.append("lua_State *L")
	if method.tag == "method" and method.get('static') is None:
		args.append(header_get_method_this(node, method))
	# w(file, f"/// @function {method.get('name')}")
	if method.findtext("summary"):
		w(file, f"/// @brief {method.findtext('summary')}")
	# if method.get("returns"):
	# 	w(file, f"/// @returns {method.get('returns')}")
	for arg in method.findall('arg'):
		args.append(header_get_method_arg(arg))
	args = ', '.join(args)
	returns = method.find('returns')
	ret = 'void' if returns is None else header_get_arg_type(returns)
	w(file, f"ORCA_API {ret}\n{header_get_method_name(node, method)}({args or 'void'});\n")

	topic = ET.SubElement(g_article, 'div', {'class':'topic'})
	doc.add_function(topic, method, returns and returns.get('type'))
	doc.add_small_abstract(topic, method.findtext('summary'))

def export_get_name(node, method):
	return f"f_{node.get('name')}_{camel_case(method.get('export') or method.get('name'))}"

def export_index_name(node):
	return f"f_{node.get('name')}___index"

def export_newindex_name(node):
	return f"f_{node.get('name')}___newindex"

def export_check_var(target, arg_type, idx, access = ''):
	if arg_type == 'STRING':
		return f"strncpy({target}, luaL_checkstring(L, {idx}), sizeof(STRING));"
	elif arg_type in atomic_types:
		check, _ = atomic_types[arg_type]
		return f"{target} = {access}{check}(L, {idx})"
	else:
		return f"{target} = {access}luaX_check{arg_type}(L, {idx})"

def export_push_var(ret, var):
	rtype = ret.get('type')
	if rtype in atomic_types:
		_, push = atomic_types[rtype]
		return f"{push}(L, {var})"
	elif ret.get('pointer') or rtype in g_enums:
		return f"luaX_push{rtype}(L, {var})"
	else:
		return f"luaX_push{rtype}(L, &{var})"

def export_write_method(node, method, file):
	args = []
	idx = 1
	if method.get('private'):
		return
	if method.get("lua"):
		args.append("L")
	w(file, f"static int {export_get_name(node, method)}(lua_State *L) {{")
	if method.tag == "method" and method.get('static') is None:
		w(file, f"\t{header_get_method_this(node, method)} = luaX_check{node.get('name')}(L, 1);")
		args.append("self")
		idx = 2
	for i, arg in enumerate(method.findall('arg')):
		arg_type = arg.get('type')
		w(file, f"\t{export_check_var(header_get_method_arg(arg), arg_type, i+idx)};")
		args.append(arg.get('name'))
	call = f"{header_get_method_name(node, method)}({', '.join(args)})"
	returns = method.find('returns')
	if returns is None:
		w(file, f"\t{call};\n\treturn 0;")
	elif returns.get('type') == 'nresults':
		w(file, f"\treturn {call};")
	else:
		ret = header_get_arg_type(returns)
		w(file, f"\t{ret} output = {call};\n\t{export_push_var(returns, 'output')};\n\treturn 1;")
	w(file, f"}}")

def function_parse(root, function, parser):
	header_write_method(root, function, parser.header)
	export_write_method(root, function, parser.export)

def write_struct_def(root, struct, h):
	sname, sbrief = struct.get('name'), struct.findtext('summary')

	global g_article
	g_article = doc.add_article(g_content, sname, struct.get('parent') or 'None', struct.findtext('xmlns') or 'None')
	atts = []
	# if struct.findtext('xmlns'): atts.append(("@xmlns ", struct.findtext('xmlns')))
	doc.add_header(g_article, struct.tag.capitalize(), sname, sbrief)
	doc.add_code(g_article, atts, "struct ", "%s.%s"%(root.get('name'),sname), struct.get('parent') or (struct.tag == 'component' and 'object'))
	doc.add_overview(g_article, struct.find('details'))

	icons = {
		'struct': 'gear',
		'component': 'layer-group',
		'interface': 'atom',
	}

	link = ET.SubElement(g_sidebar, 'a', {'class':'leaf-link', 'href':f'#{sname}'})
	ET.SubElement(link, 'i', {'class':f'fa-solid fa-{icons.get(struct.tag)} icon'})
	ET.SubElement(link, 'span').text = sname+'\n'

	if struct.findall('property') or struct.findall('methods'):
		ET.SubElement(g_article, 'h2', {'class':'title'}).text = 'Properties'

	if struct.tag == "interface":
		return

	if sbrief: w(h, f"/// @brief {sbrief}")
	w(h, f"struct {sname} {{")
	for field in struct.findall('property')+struct.findall('field'):
		fname, ftype = field.get('name'), header_get_arg_type(field)
		fcomment = f" /// {field.text}" if field.text else str()
		if field.get('array'):
			w(h, f"\t{ftype} {fname}[{field.get('array')}];{fcomment}")
		else:
			w(h, f"\t{ftype} {fname};{fcomment}")
		decor = 'var'
		ftype = field.get('type')
		if ftype in g_enums:
			decor = 'enum'
		if ftype in g_components:
			decor = 'object'
		if ftype in g_resources:
			decor = 'resource'
		if field.tag == 'field':# or struct.tag == 'struct':
			topic = ET.SubElement(g_article, 'div', {'class':'topic'})
			topic.tail = "\n"
			doc.add_property(topic, decor, fname, ftype)
			doc.add_small_abstract(g_article, field.text)
	w(h, f"}};\n")

def write_struct_fwd_def(sname, typedef, parser):
	w(parser, f"typedef struct {sname} {_t(typedef)}, *{lp(typedef)};")
	w(parser, f"typedef struct {sname} const {_c(typedef)}, *{lpc(typedef)};")
	# w(parser, f"FWD_STRUCT({sname});")

def struct_parse(root, struct, parser):
	"""Parse a struct and generate bindings"""
	write_struct_def(root, struct, parser.header)
	
	# Use ExportPlugin methods for all export code generation
	if parser.export_plugin:
		parser.export_plugin.write_struct_bindings(root, struct)
		
		# write methods
		for method in struct.findall('method'):
			function_parse(struct, method, parser)
		
		# write __index function
		parser.export_plugin.write_index_function(root, struct)
		
		# write __newindex function
		parser.export_plugin.write_newindex_function(root, struct)
		
		# write lua_open function
		parser.export_plugin.write_luaopen_function(root, struct)

def enums_parse(_, enums, parser):
	e, h = parser.export, parser.header
	ename = enums.get('name')

	article = doc.add_article(g_content, ename, 'None', 'None')
	doc.add_header(article, "Enum", ename, enums.findtext('summary'))
	# doc.add_code(article, atts, "struct ", "%s.%s"%(root.get('name'),sname), struct.get('parent'))
	doc.add_overview(article, enums.find('details'))

	link = ET.SubElement(g_sidebar, 'a', {'class':'leaf-link', 'href':f'#{ename}'})
	ET.SubElement(article, 'h2', {'class':'title'}).text = 'Constants'
	ET.SubElement(link, 'i', {'class':f'fa-solid fa-bars icon'})
	ET.SubElement(link, 'span').text = ename+'\n'

	values = ['"%s"'%e.get('name').lower() for e in enums.findall('enum')]+["NULL"]
	w(e, f"static const char *_{ename}[] = {{{','.join(values)}}};")
	w(e, f"{_e(ename)} luaX_check{ename}(lua_State *L, int idx) {{")
	w(e, f"\treturn luaL_checkoption(L, idx, NULL, _{ename});")
	w(e, f"}}")
	w(e, f"void luaX_push{ename}(lua_State *L, {_e(ename)} value) {{")
	w(e, f"\tassert(value >= 0 && value < {len(enums.findall('enum'))});")
	w(e, f"\tlua_pushstring(L, _{ename}[value]);")
	w(e, f"}}")

	w(h, f"typedef enum {enums.get('name')} {{")
	for enum in enums.findall('enum'):
		enum_name = f"k{enums.get('name')+enum.get('name')}"
		if enum.text:
			w(h, f"\t{enum_name}, /// {enum.text}")
		else:
			w(h, f"\t{enum_name},")

		topic = ET.SubElement(article, 'div', {'class':'topic'})
		topic.tail = "\n"
		doc.add_property(topic, 'case', enum.get('name'), '')
		doc.add_small_abstract(topic, enum.text)

	w(h, f"}} {_e(enums.get('name'))};\n")

	g_dtd.write(f"<!ENTITY % {enums.get('name')} \"{'|'.join([e.get('name') for e in enums.findall('enum')])}\">\n")

def property_name(name):
	def conv_name(name):
		for pat, repl in Axis:
			s2, n = pat.subn(repl, name, count=1)
			if n: return s2
		return name
	name = conv_name(name)
	return "".join([s[0].upper() + s[1:] for s in name.split('.')])

def struct_property(root, cmp, struct, parser, prefix, indent):
	n = 0
	for shorthand in struct.findall("shorthand"):
		cname = cmp.get('name')
		pname, physical, userdata = shorthand.get('name'), shorthand.get('physical'), shorthand.get('userdata')
		if parser:
			e, h, s = parser.export, parser.header, parser.props
			w(e, f"{indent}\t/* {cname}.{pname} */ DECL({hash(pname)}, {hash(cname+'.'+pname)},\n{indent}\t{cname}, \"{pname}\", {physical}, kDataTypeEdges, .TypeString=\"{userdata}\"),")
			w(s, f"{indent}\tk{cname}{pname},")
		n = n + 1
	for field in struct.findall('field') if not struct.get('sealed') else []:
		if field.get('array'):
			for m in range(int(field.get('array'))):
				name = '%s[%d]'%(field.get('name'), m)
				n = n + component_property(root, cmp, prefix+name, field, parser, indent+'\t')
		else:
			n = n + component_property(root, cmp, prefix+field.get('name'), field, parser, indent+'\t')
	return n

def component_property(root, cmp, path, property, parser, indent):
	sname = property_name(path)
	ptype, cname = property.get('type'), cmp.get('name')
	struct = g_structs.get(ptype)
	enum = None
	decor = 'var'
	n = 0
	if not property.get('exclude-self'):
		# if ptype.endswith("_t"):
		# 	typedata = f"T_{ptype[:-2].upper()}"
		# else:
		typedata = f"kDataType{ptype[:1].upper()+ptype[1:]}"
		if struct:
			export_as = struct.get('export')
			nfields = struct_property(root, cmp, struct, None, str(), indent)
			typedata = f"kDataType{export_as}" if export_as else f"kDataTypeGroup, .TypeString=\"{ptype}\", .NumComponents={nfields}"
			decor = 'struct'
		elif ptype in g_enums or sname in g_enums:
			enum = g_enums.get(ptype) or g_enums.get(sname)
			values = [e.get('name') for e in enum.findall('enum')]
			ptype = enum.get('name')
			typedata = f"kDataTypeEnum, .TypeString=\"{','.join(values)}\""
			decor = 'enum'
		# elif ptype in g_resources:
		# 	typedata = f"T_HANDLE, .TypeString=\"{ptype.capitalize()}\""
		# 	decor = 'resource'
		elif ptype in g_components:
			typedata = f"kDataTypeObject, .TypeString=\"{ptype}\""
			decor = 'object'
		if parser:
			w(parser.export, f"{indent}/* {cname}.{sname} */ DECL({hash(sname)}, {hash(cname+'.'+sname)},\n{indent}{cname}, \"{sname}\", {path}, {typedata}),")
			w(parser.props, f"\tk{cname}{sname},")
			g_attribs.append(sname)
			if ptype in g_enums:
				g_dtd.write(f"\t{sname} (%{ptype};) #IMPLIED\n")
			elif ptype == 'bool':
				g_dtd.write(f"\t{sname} (true|false) #IMPLIED\n")
			else:
				g_dtd.write(f"\t{sname} CDATA #IMPLIED\n")

			topic = ET.SubElement(g_article, 'div', {'class':'topic'})
			topic.tail = "\n"
			args = path.split('.')
			args.reverse()
			args.pop(0)

			doc.add_property(topic, decor, sname, ptype)
			doc.add_small_abstract(g_article, property.text and property.text.format(*args))

			# if enum:
			# 	doc.add_small_abstract(g_article, ', '.join(values))

		n = 1
	return (struct_property(root, cmp, struct, parser, path+'.', indent) + n) if struct else n

def component_get_parents(component):
	parents = []
	if component.get('parent'):
		parents.append(component.get('parent'))
	if component.get('concept'):
		parents.append(component.get('concept'))
	return parents

def component_parse(root, component, parser):
	e, h, s = parser.export, parser.header, parser.props
	cname = component.get('name')

	g_dtd.write(f"<!ENTITY % {component.get('name')}Attribs \"\n")
	g_attribs.clear()

	if component.get('parent'):
		g_dtd.write(f"\t%{component.get('parent')}Attribs;\n");
		if component.get('concept'):
			g_dtd.write(f"\t%{component.get('concept')}Attribs;\n");
	else:
		g_dtd.write(f"\tName CDATA #REQUIRED\n");
		g_dtd.write(f"\tStyle CDATA #IMPLIED\n");
		g_dtd.write(f"\tIsDisabled (true|false) #IMPLIED\n");
		g_dtd.write(f"\tStateManager CDATA #IMPLIED\n");

	w(h, f"typedef struct {cname} {cname}, *{cname}Ptr;")
	w(h, f"typedef struct {cname} const *{cname}CPtr;")
	write_struct_def(root, component, parser.header)
	for method in component.findall('method'):
		header_write_method(component, method, parser.header)
		# write methods
	for handles in component.findall('handles'):
		event = handles.get('event')
		w(e, f"LRESULT {cname}_{event}({lpobject_t}, {lp(cname)}, wParam_t, {event}EventPtr);")
	w(e, f"static struct PropertyDesc const {cname}Properties[k{cname}NumProperties] = {{")
	w(s, f"#define ID_{cname} " + (hash(cname)))
	w(s, f"#define Get{cname}(_P)((struct {cname}*)((_P)?OBJ_GetComponent(_P,ID_{cname}):NULL))")
	w(s, f"#define {cname}_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_{cname},sizeof(struct {cname}),_N)")

	w(s, f"enum {cname}Properties {{")

	parser.props = io.StringIO()

	for shorthand in component.findall("shorthand"):
		pname, physical, userdata = shorthand.get('name'), shorthand.get('physical'), shorthand.get('userdata')
		w(e, f"\t/* {cname}.{cname} */ DECL({hash(pname)}, {hash(cname+'.'+pname)},\n\t{cname}, \"{pname}\", {physical}, T_EDGES, .TypeString=\"{userdata}\"),")
		w(parser.props, f"\tk{cname}{pname},")
	for property in component.findall("property"):
		pname = property.get('name')
		if property.get('array'):
			for n in range(int(property.get('array'))):
				component_property(root, component, '%s[%d]'%(pname,n), property, parser, '\t')
		else:
			component_property(root, component, pname, property, parser, '\t')

	parser.props.seek(0)
	enums = parser.props.read()
	s.write(enums)
	w(s, f"\tk{cname}NumProperties\n}};\n")
	w(e, f"}};")
	for v in enums.split(","): 
		crop = v.strip()[len(cname)+1:]
		if crop:
			w(s, f"#define ID_{cname}_{crop} {hash(cname+'.'+crop)}")

	parser.props = s

	hname = "InitDefaults"
	if bool(component.findall("property[@default]")):
		w(e, f"static struct {cname} {cname}Defaults = {{")
		for property in component.findall("property[@default]"):
			pname, ptype, pdefault = property.get('name'), property.get('type'), property.get('default')
			if ptype in g_enums and not pdefault.lstrip('-').isdigit():
				w(e, f"\t.{pname} = k{ptype}{pdefault},")
			elif ptype in g_structs:
				w(e, f"\t.{pname} = (struct {ptype}) {{ {', '.join(pdefault.split())} }},")
			elif ptype == 'bool':
				w(e, f"\t.{pname} = {pdefault.upper()},")
			elif ptype == 'fixed':
				w(e, f"\t.{pname} = \"{pdefault}\",")
			else:
				w(e, f"\t.{pname} = {pdefault},")
		w(e, f"}};")
	elif bool(component.findall("property")):
		w(e, f"static struct {cname} {cname}Defaults = {{0}};")
	else:
		w(e, f"static struct {cname} {cname}Defaults = {{}};")
	w(e, f"LRESULT {cname}Proc({lpobject_t} object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {{")
	w(e, f"\tswitch (message) {{")
	for handles in component.findall('handles'):
		hname = handles.get('event')
		w(e, f"\t\tcase {hash(hname)}: // {hname}")
		w(e, f"\t\t\treturn {cname}_{hname}(object, cmp, wparm, lparm);")			
	w(e, f"}}\n\treturn FALSE;\n}}")
	w(e, f"void luaX_push{cname}(lua_State *L, {lpc(cname)} {cname}) {{")
	w(e, f"\tluaX_pushObject(L, CMP_GetObject({cname}));\n}}")	
	w(e, f"{lp(cname)} luaX_check{cname}(lua_State *L, int idx) {{")
	w(e, f"\treturn Get{cname}(luaX_checkObject(L, idx));\n}}")	
	parents = component_get_parents(component)
	for p in parents:
		w(e, f"extern struct ClassDesc _{p};")
	parents = ", ".join([f"&_{p}" for p in parents]+["NULL"])
	content = component.get('parent', cname)
	if component.get('name').endswith('Library'):
		content = component.get('name').replace('Library', '')
	w(e, f"ORCA_API struct ClassDesc _{cname} = {{")
	w(e, f"\t.ClassName = \"{cname}\",")
	w(e, f"\t.DefaultName = \"{component.get('default', cname)}\",")
	w(e, f"\t.ContentType = \"{component.get('content', content)}\",")
	w(e, f"\t.Xmlns = \"{component.findtext('xmlns')}\",")
	w(e, f"\t.ParentClasses = {{{parents}}},")
	w(e, f"\t.ClassID = ID_{cname},")
	w(e, f"\t.ClassSize = sizeof(struct {cname}),")
	w(e, f"\t.Properties = {cname}Properties,")
	w(e, f"\t.ObjProc = {cname}Proc,")
	w(e, f"\t.Defaults = &{cname}Defaults,")
	w(e, f"\t.NumProperties = k{cname}NumProperties,\n}};");

	g_dtd.write("\">\n")

	def is_(comp, _2d):
		if comp.get('name')=="Viewport3D":
			return False
		if comp and _2d in comp.get('name'):
			return True
		if comp.get('parent') is None:
			return False
		return is_(g_components.get(comp.get('parent')), _2d)

	for att in g_attribs:
		g_dtd.write(f"<!ELEMENT {cname}.{att} (#PCDATA | Binding)*>\n")
		g_dtd.write(f"<!ATTLIST {cname}.{att} Mode (expression|oneway) #IMPLIED Attribute CDATA #IMPLIED Enabled (true|false) #IMPLIED>\n")

	attribs = [f"{cname}.{att}" for att in g_attribs]

	if component.get("parent"):
		attribs.append(f"%{component.get('parent')}Elements;")

	if component.get("concept"):
		attribs.append(f"%{component.get('concept')}Elements;")

	g_dtd.write(f"<!ENTITY % {cname}Elements \"{'|'.join(attribs)}\">\n")

	g_dtd.write(f"<!ELEMENT {cname} ")
	elm = f"%{cname}Elements;"
	if is_(component, '2D'):
		arr = [key for key, value in g_components.items() if is_(value, "2D")]+["Viewport3D","Resource","StyleSheet","LayerPrefabPlaceholder",elm]
		g_dtd.write(f"({'|'.join(arr)})*>\n")
	elif is_(component, '3D'):
		arr = [key for key, value in g_components.items() if is_(value, "3D")]+["Resource","StyleSheet","ObjectPrefabPlaceholder",elm]
		g_dtd.write(f"({'|'.join(arr)})*>\n")
	elif component.get('name')=="Viewport3D":
		arr = [key for key, value in g_components.items()]+["Resource","StyleSheet","ObjectPrefabPlaceholder",elm]
		g_dtd.write(f"({'|'.join(arr)})*>\n")
	else:
		g_dtd.write("EMPTY>\n")
	g_dtd.write(f"<!ATTLIST {cname} %{cname}Attribs;>\n")

def include_parse(_, include, parser):
	w(parser.header, f"#include <{include.get('file')}>")

def write_push_check(name, h):
	w(h, f"/// @brief Push {name} onto Lua stack.")
	w(h, f"ORCA_API void\nluaX_push{name}(lua_State *L, {lpc(name)} {name});")
	w(h, f"/// @brief Check {name} form Lua stack at index.")
	w(h, f"ORCA_API {lp(name)}\nluaX_check{name}(lua_State *L, int idx);\n")

def external_parse(_, external, parser):
	sname = external.get('struct')
	write_struct_fwd_def(sname, sname, parser.header)

def resource_parse(_, resource, parser):
	name = resource.get('type')
	write_struct_fwd_def(name, name, parser.header)
	if resource.get('no-lua'):
		return
	write_push_check(name, parser.header)
	w(parser.export, f"void luaX_push{name}(lua_State *L, {lpc(name)} {name}) {{")
	w(parser.export, f"\tlua_pushlightuserdata(L, ({lp(name)}){name});\n}}")	
	w(parser.export, f"{lp(name)} luaX_check{name}(lua_State *L, int idx) {{")
	w(parser.export, f"\treturn lua_touserdata(L, idx);\n}}")	

def event_parse(_, event, parser):
	ename, etype = event.get('name'), event.get('type')
	w(parser.header, f"#define kEvent{ename} " + hash(ename))	
	if etype == "void":
		w(parser.header, f"typedef void* {ename}EventPtr;\n")
	elif etype:
		w(parser.header, f"typedef struct {etype}* {ename}EventPtr;\n")

global_parsers={
	'struct': struct_parse,
	'interface': struct_parse,
	'function': function_parse,
	'enums': enums_parse,
	'component': component_parse,
	'include': include_parse,
	'external': external_parse,
	'resource': resource_parse,
	'event': event_parse,
}

def read_xml(filename):
	if not os.path.exists(filename):
		print(f"File not found: {filename}")
		return
	tree = ET.parse(filename)
	root = tree.getroot()
	parser = ParserState(filename)
	subpath = filename[filename.index("source/"):]

	if root.tag != "module":
		print("Expected 'module' tag")
		return

	tech = ET.SubElement(g_sidebar, 'div', {'class':'technology-title'})
	i = ET.SubElement(tech, 'span', {'class':'fa-solid fa-folder-open icon'})
	i.tail = f"{root.get('namespace')}.{root.get('name')}\n"

	h, e = parser.header, parser.export

	w(h, f"// Generated by codegen utility")
	w(h, f"#ifndef __{root.get('name').upper()}_H__")
	w(h, f"#define __{root.get('name').upper()}_H__\n")
	w(h, f"#include <include/orca.h>\n")

	w(e, f"// Generated by codegen utility")
	w(e, f"#include <include/api.h>")
	w(e, f"#include <{subpath.replace('.xml', '.h')}>")

	if root.find('component') is not None:
		w(h, f"#include <{subpath.replace('.xml', '_properties.h')}>\n")
		w(e, "#define DECL(SHORT, LONG, CLASS, NAME, FIELD, TYPE,...) { \\")
		w(e, "\t.id=&(struct ID){.Name=#CLASS\".\"NAME,.Identifier=SHORT}, \\")
		# w(e, "\t.ShortIdentifier=SHORT, \\")
		w(e, "\t.FullIdentifier=LONG, \\")
		# w(e, "\t.Name=NAME, \\")
		w(e, "\t.Offset=offsetof(struct CLASS, FIELD), \\")
		w(e, "\t.DataSize=sizeof(((struct CLASS *)NULL)->FIELD), \\")
		w(e, "\t.DataType=TYPE, ##__VA_ARGS__ }")

	w(e, "")

	name, namespace = root.get('name'), root.get('namespace')

	g_structs.update({struct.get('name'): struct for struct in root.findall(f".//struct[@name]")})
	g_enums.update({enum.get('name'): enum for enum in root.findall(f".//enums[@name]")})
	g_components.update({cmp.get('name'): cmp for cmp in root.findall(f".//component[@name]")})
	g_resources.update({cmp.get('type'): cmp for cmp in root.findall(f".//resource[@type]")})
	
	# Set globals on plugins
	parser.set_globals(g_structs, g_enums, g_components, g_resources)

	for struct in root.findall('struct')+root.findall('interface')+root.findall('component'):
		struct_name = struct.get('name')
		write_struct_fwd_def(struct_name, struct_name, parser.header)
		write_push_check(struct_name, parser.header)

	for node in root:
		global_parsers.get(node.tag, throw_no_parser)(root, node, parser)

	for shutdown in root.findall('shutdown'):
		fn = shutdown.get('name')
		w(e, f"static int f_{name}_gc(lua_State *L) {{\n\tvoid {fn}(void);\n\t{fn}();\n\treturn 0;\n}}")

	# write module luaopen 
	w(e, f"ORCA_API int luaopen_{namespace}_{name}(lua_State *L) {{") 
	w(e, f"\tluaL_newlib(L, ((luaL_Reg[]) {{")
	for fn in root.findall('function'):
		w(e, f"\t\t{{ \"{camel_case(fn.get('name'))}\", {export_get_name(root, fn)} }},")
	w(e, f"\t\t{{ NULL, NULL }}")
	w(e, f"\t}}));")
	if root.get('on-luaopen'):
		w(e, f"\tvoid {root.get('on-luaopen')}(lua_State *L);")
		w(e, f"\t{root.get('on-luaopen')}(L);")
	for struct in root.findall('struct')+root.findall('interface'):
		str_name = struct.get('name')
		w(e, f"\t// {str_name}")
		w(e, f"\tluaopen_{namespace}_{str_name}(L);") 
		w(e, f"\tlua_setfield(L, -2, \"{struct.get('export', str_name)}\");") 
	for component in root.findall('component'):
		cmp_name = component.get('name')
		w(e, f"\t// {cmp_name}")
		w(e, f"\tlua_pushclass(L, &_{cmp_name});")
		w(e, f"\tlua_setfield(L, -2, \"{cmp_name}\");")
	for shutdown in root.findall('shutdown'):
		w(e, f"\tAPI_MODULE_SHUTDOWN(L, f_{name}_gc);")
	w(e, f"\treturn 1;\n}}") 

	w(h, f"#endif")
  
if __name__ == "__main__":
	for i in range(len(sys.argv)-1):
		read_xml(sys.argv[i+1])

	tree = ET.ElementTree(g_html)
	# ET.indent(tree, space="\t", level=0)
	tree.write(g_output, encoding="utf-8", method="html")

	with open("schemas/append.dtd") as f:
		g_dtd.write(f.read())

	# _b = ET.SubElement(g_xsd, "xs:simpleType", {"name":"Attribute"})
	# _b = ET.SubElement(_b, "xs:restriction", {"base":"xs:string"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"VECTOR_X"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"VECTOR_Y"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"VECTOR_Z"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"VECTOR_W"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"COLOR_R"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"COLOR_G"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"COLOR_B"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"COLOR_A"})
	# ET.SubElement(_b, "xs:enumeration", {"value":"WHOLE_PROPERTY"})

	# _b = ET.SubElement(g_xsd, "xs:complexType", {"name":"Binding"})
	# ET.SubElement(_b, "xs:attribute", {"name":"Enabled", "type":"xs:boolean"})
	# ET.SubElement(_b, "xs:attribute", {"name":"Mode", "type":"xs:string"})
	# ET.SubElement(_b, "xs:attribute", {"name":"Property", "type":"xs:string"})
	# ET.SubElement(_b, "xs:attribute", {"name":"Attribute", "type":"Attribute"})

	# ET.SubElement(g_xsd, "xs:element", {"name":"Binding","type":"Binding"})

