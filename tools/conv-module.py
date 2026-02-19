import sys, re, io, os
import xml.etree.ElementTree as ET

g_structs = {}
g_enums = {}
g_components = {}
g_resources = {}

class HtmlWriter:
	def __init__(self):
		self.article = None
		self.html = ET.Element('html', attrib={'xmlns':'http://www.w3.org/2001/XMLSchema' })
		self.head = ET.SubElement(self.html, 'head')
		ET.SubElement(self.head, 'link', { "rel":"stylesheet", "href":"doc.css" })
		ET.SubElement(self.head, 'link', { "rel":"stylesheet", "href":"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css" })
		ET.SubElement(self.head, 'meta', {'charset':"UTF-8"})
		self.body = ET.SubElement(self.html, 'body')
		self.wrapper = ET.SubElement(self.body, 'div', {'class': 'adjustable-sidebar-width full-width-container topic-wrapper'})
		self.sidebar = ET.SubElement(self.wrapper, 'div', {'class': 'sidebar'})
		self.content = ET.SubElement(self.wrapper, 'main', {'class': 'content'})
		self.output = "../docs/index.html"

	def open(self, _, root):		
		tech = ET.SubElement(self.sidebar, 'div', {'class':'technology-title'})
		i = ET.SubElement(tech, 'span', {'class':'fa-solid fa-folder-open icon'})
		i.tail = f"{root.get('namespace')}.{root.get('name')}\n"

	def finalize(self):
		tree = ET.ElementTree(self.html)
		# ET.indent(tree, space="\t", level=0)
		tree.write(self.output, encoding="utf-8", method="html")

	def on_struct(self, root, struct):
		sname, sbrief = struct.get('name'), struct.findtext('summary')

		self.article = doc.add_article(self.content, sname, struct.get('parent') or 'None', struct.findtext('xmlns') or 'None')
		atts = []
		# if struct.findtext('xmlns'): atts.append(("@xmlns ", struct.findtext('xmlns')))
		doc.add_header(self.article, struct.tag.capitalize(), sname, sbrief)
		doc.add_code(self.article, atts, "struct ", "%s.%s"%(root.get('name'),sname), struct.get('parent') or (struct.tag == 'component' and 'object'))
		doc.add_overview(self.article, struct.find('details'))

		icons = {
			'struct': 'gear',
			'component': 'layer-group',
			'interface': 'atom',
		}

		link = ET.SubElement(self.sidebar, 'a', {'class':'leaf-link', 'href':f'#{sname}'})
		ET.SubElement(link, 'i', {'class':f'fa-solid fa-{icons.get(struct.tag)} icon'})
		ET.SubElement(link, 'span').text = sname+'\n'

		if struct.findall('property') or struct.findall('methods'):
			ET.SubElement(self.article, 'h2', {'class':'title'}).text = 'Properties'

		for field in struct.findall('property')+struct.findall('field'):
			fname, ftype = field.get('name'), header_get_arg_type(field)
			decor = 'var'
			ftype = field.get('type')
			if ftype in g_enums: decor = 'enum'
			if ftype in g_components: decor = 'object'
			if ftype in g_resources: decor = 'resource'
			if field.tag == 'field':# or struct.tag == 'struct':
				topic = ET.SubElement(self.article, 'div', {'class':'topic'})
				topic.tail = "\n"
				doc.add_property(topic, decor, fname, ftype)
				doc.add_small_abstract(self.article, field.text)

		for method in struct.findall('method'):
			self.on_function(struct, method)

	def on_function(self, _, method):
		returns = method.find('returns')
		topic = ET.SubElement(self.article, 'div', {'class':'topic'})
		doc.add_function(topic, method, returns and returns.get('type'))
		doc.add_small_abstract(topic, method.findtext('summary'))

	def on_property(self, property, _, path):
		property_type = property.get('type')
		readable_name = property.get('name') if property.tag == 'shorthand' else property_name(path)
		struct = g_structs.get(property_type)
		decor = 'var'
		if property_type == 'Edges': 
			return
		if struct is not None:
			decor = 'struct'
		elif property_type in g_enums or readable_name in g_enums:
			enum = g_enums.get(property_type) or g_enums.get(readable_name)
			property_type = enum.get('name')
			decor = 'enum'
		elif property_type in g_components:
			decor = 'object'

		topic = ET.SubElement(self.article, 'div', {'class':'topic'})
		topic.tail = "\n"
		args = path.split('.')
		args.reverse()
		args.pop(0)

		doc.add_property(topic, decor, readable_name, property_type)
		doc.add_small_abstract(self.article, property.text and property.text.format(*args))

		# if enum: doc.add_small_abstract(g_article, ', '.join(values))

	def on_component(self, root, component):
		self.on_struct(root, component)
		enum_component_properties(component, self.on_property)

	def on_enums(self, root, enums):
		ename = enums.get('name')
		article = doc.add_article(self.content, ename, 'None', 'None')
		doc.add_header(article, "Enum", ename, enums.findtext('summary'))
		# doc.add_code(article, atts, "struct ", "%s.%s"%(root.get('name'),sname), struct.get('parent'))
		doc.add_overview(article, enums.find('details'))

		link = ET.SubElement(self.sidebar, 'a', {'class':'leaf-link', 'href':f'#{ename}'})
		ET.SubElement(article, 'h2', {'class':'title'}).text = 'Constants'
		ET.SubElement(link, 'i', {'class':f'fa-solid fa-bars icon'})
		ET.SubElement(link, 'span').text = ename+'\n'

		for enum in enums.findall('enum'):
			topic = ET.SubElement(article, 'div', {'class':'topic'})
			topic.tail = "\n"
			doc.add_property(topic, 'case', enum.get('name'), '')
			doc.add_small_abstract(topic, enum.text)

	def on_interface(self, *args):
		return self.on_struct(*args)

class DTDWriter:
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
			self.write("<!ATTLIST "
					f"{cname}.{sname} Mode (expression|oneway) #IMPLIED "
					"Attribute CDATA #IMPLIED Enabled (true|false) #IMPLIED>\n")

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

class HeaderWriter:
	def __init__(self):
		self.file = None

	def open(self, xml, root):
		base, _ = os.path.splitext(xml)
		self.file = open(base + ".h", "w")

		self.w(f"// Generated by codegen utility")
		self.w(f"#ifndef __{root.get('name').upper()}_H__")
		self.w(f"#define __{root.get('name').upper()}_H__\n")
		self.w(f"#include <include/orca.h>\n")

		if root.find('component') is not None:
			self.w(f"#include <{base[base.index("source/"):] + '_properties.h'}>\n")

	def w(self, text):
		self.file.write(text)
		self.file.write("\n")

	def write(self, text):
		self.file.write(text)

	def close(self):
		self.w(f"#endif")
		self.file.close()

	def struct_fwd_def(self, sname):
		self.w(f"typedef struct {sname} {_t(sname)}, *{lp(sname)};")
		self.w(f"typedef struct {sname} const {_c(sname)}, *{lpc(sname)};")
		# self.w(f"FWD_STRUCT({sname});")

	def lua_accessors(self, name):
		self.w(f"/// @brief Push {name} onto Lua stack.")
		self.w(f"ORCA_API void\nluaX_push{name}(lua_State *L, {lpc(name)} {name});")
		self.w(f"/// @brief Check {name} form Lua stack at index.")
		self.w(f"ORCA_API {lp(name)}\nluaX_check{name}(lua_State *L, int idx);\n")

	def on_external(self, _, external):
		sname = external.get('struct')
		self.struct_fwd_def(sname)

	def on_resource(self, _, resource):
		name = resource.get('type')
		self.struct_fwd_def(name)
		if resource.get('no-lua'):
			return
		self.lua_accessors(name)

	def on_include(self, _, include):
		self.w(f"#include <{include.get('file')}>")

	def on_event(self, _, event):
		ename, etype = event.get('name'), event.get('type')
		self.w(f"#define kEvent{ename} " + hash(ename))	
		if etype == "void":
			self.w(f"typedef void* {ename}EventPtr;\n")
		elif etype:
			self.w(f"typedef struct {etype}* {ename}EventPtr;\n")

	def on_function(self, node, function):
		args = []
		if function.get("lua"):
			args.append("lua_State *L")
		if function.tag == "method" and function.get('static') is None:
			args.append(header_get_method_this(node, function))
		# self.w(f"/// @function {function.get('name')}")
		if function.findtext("summary"):
			self.w(f"/// @brief {function.findtext('summary')}")
		# if function.get("returns"):
		# 	self.w(f"/// @returns {function.get('returns')}")
		for arg in function.findall('arg'):
			args.append(header_get_method_arg(arg))
		args = ', '.join(args)
		returns = function.find('returns')
		ret = 'void' if returns is None else header_get_arg_type(returns)
		self.w(f"ORCA_API {ret}\n{header_get_method_name(node, function)}({args or 'void'});\n")

	def on_struct(self, _, struct):
		sname, sbrief = struct.get('name'), struct.findtext('summary')
		if sbrief: self.w(f"/// @brief {sbrief}")
		self.w(f"struct {sname} {{")
		for field in struct.findall('property')+struct.findall('field'):
			fname, ftype = field.get('name'), header_get_arg_type(field)
			fcomment = f" /// {field.text}" if field.text else str()
			if field.get('array'):
				self.w(f"\t{ftype} {fname}[{field.get('array')}];{fcomment}")
			else:
				self.w(f"\t{ftype} {fname};{fcomment}")
		self.w(f"}};\n")
		for method in struct.findall('method'):
			self.on_function(struct, method)

	def on_component(self, root, component):
		cname = component.get('name')
		self.w(f"typedef struct {cname} {cname}, *{cname}Ptr;")
		self.w(f"typedef struct {cname} const *{cname}CPtr;")
		self.on_struct(root, component)

	def on_interface(self, _, interface):
		for method in interface.findall('method'):
			self.on_function(interface, method)

	def on_enums(self, _, enums):
		self.w(f"typedef enum {enums.get('name')} {{")
		for enum in enums.findall('enum'):
			enum_name = f"k{enums.get('name')+enum.get('name')}"
			if enum.text:
				self.w(f"\t{enum_name}, /// {enum.text}")
			else:
				self.w(f"\t{enum_name},")
		self.w(f"}} {_e(enums.get('name'))};\n")

class PropertiesWriter:
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
		self.w(f"#define ID_{cname}_{name} {hash(cname+'.'+name)}")

	def on_component(self, root, component):		
		cname = component.get('name')
		self.w(f"#define ID_{cname} " + (hash(cname)))
		self.w(f"#define Get{cname}(_P)((struct {cname}*)((_P)?OBJ_GetComponent(_P,ID_{cname}):NULL))")
		self.w(f"#define {cname}_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_{cname},sizeof(struct {cname}),_N)")
		self.w(f"enum {cname}Properties {{")
		enum_component_properties(component, self.write_property_enum)
		self.w(f"\tk{cname}NumProperties\n}};\n")
		enum_component_properties(component, self.write_property_id)

class ExportWriter:
	def __init__(self):
		self.file = None

	def open(self, xml, root):
		base, _ = os.path.splitext(xml)
		self.root, self.name, self.namespace = root, root.get('name'), root.get('namespace')
		self.file = open(base + "_export.c", "w")
		self.w(f"// Generated by codegen utility")
		self.w(f"#include <include/api.h>")
		self.w(f"#include <{base[base.index("source/"):] + '.h'}>")

		if root.find('component') is not None:
			self.w(f"#define DECL(SHORT, LONG, CLASS, NAME, FIELD, TYPE,...) {{ \\")
			self.w(f"\t.id=&(struct ID){{.Name=#CLASS\".\"NAME,.Identifier=SHORT}}, \\")
			# self.w(f"\t.ShortIdentifier=SHORT, \\")
			self.w(f"\t.FullIdentifier=LONG, \\")
			# self.w(f"\t.Name=NAME, \\")
			self.w(f"\t.Offset=offsetof(struct CLASS, FIELD), \\")
			self.w(f"\t.DataSize=sizeof(((struct CLASS *)NULL)->FIELD), \\")
			self.w(f"\t.DataType=TYPE, ##__VA_ARGS__ }}")

		self.w(f"")

	def close(self):
		# write module luaopen 
		self.w(f"ORCA_API int luaopen_{self.namespace}_{self.name}(lua_State *L) {{") 
		self.w(f"\tluaL_newlib(L, ((luaL_Reg[]) {{")
		for fn in self.root.findall('function'):
			self.w(f"\t\t{{ \"{camel_case(fn.get('name'))}\", {export_get_name(self.root, fn)} }},")
		self.w(f"\t\t{{ NULL, NULL }}")
		self.w(f"\t}}));")
		if self.root.get('on-luaopen'):
			self.w(f"\tvoid {self.root.get('on-luaopen')}(lua_State *L);")
			self.w(f"\t{self.root.get('on-luaopen')}(L);")
		for struct in self.root.findall('struct')+self.root.findall('interface'):
			str_name = struct.get('name')
			self.w(f"\t// {str_name}")
			self.w(f"\tluaopen_{self.namespace}_{str_name}(L);") 
			self.w(f"\tlua_setfield(L, -2, \"{struct.get('export', str_name)}\");") 
		for component in self.root.findall('component'):
			cmp_name = component.get('name')
			self.w(f"\t// {cmp_name}")
			self.w(f"\tlua_pushclass(L, &_{cmp_name});")
			self.w(f"\tlua_setfield(L, -2, \"{cmp_name}\");")
		for _ in self.root.findall('shutdown'):
			self.w(f"\tAPI_MODULE_SHUTDOWN(L, f_{self.name}_gc);")
		self.w(f"\treturn 1;\n}}") 

	def w(self, text):
		self.file.write(text)
		self.file.write("\n")

	def on_shutdown(self, _, shutdown):
		fn = shutdown.get('name')
		self.w(f"static int f_{self.name}_gc(lua_State *L) {{\n\tvoid {fn}(void);\n\t{fn}();\n\treturn 0;\n}}")

	def on_enums(self, _, enums):
		ename = enums.get('name')

		values = ['"%s"'%e.get('name').lower() for e in enums.findall('enum')]+["NULL"]
		self.w(f"static const char *_{ename}[] = {{{','.join(values)}}};")
		self.w(f"{_e(ename)} luaX_check{ename}(lua_State *L, int idx) {{")
		self.w(f"\treturn luaL_checkoption(L, idx, NULL, _{ename});")
		self.w(f"}}")
		self.w(f"void luaX_push{ename}(lua_State *L, {_e(ename)} value) {{")
		self.w(f"\tassert(value >= 0 && value < {len(enums.findall('enum'))});")
		self.w(f"\tlua_pushstring(L, _{ename}[value]);")
		self.w(f"}}")

	def on_resource(self, _, resource):
		if resource.get('no-lua'):
			return
		name = resource.get('type')
		self.w(f"void luaX_push{name}(lua_State *L, {lpc(name)} {name}) {{")
		self.w(f"\tlua_pushlightuserdata(L, ({lp(name)}){name});\n}}")	
		self.w(f"{lp(name)} luaX_check{name}(lua_State *L, int idx) {{")
		self.w(f"\treturn lua_touserdata(L, idx);\n}}")	

	def on_function(self, node, method):
		args = []
		idx = 1
		if method.get('private'):
			return
		if method.get("lua"):
			args.append("L")
		self.w(f"static int {export_get_name(node, method)}(lua_State *L) {{")
		if method.tag == "method" and method.get('static') is None:
			self.w(f"\t{header_get_method_this(node, method)} = luaX_check{node.get('name')}(L, 1);")
			args.append("self")
			idx = 2
		for i, arg in enumerate(method.findall('arg')):
			arg_type = arg.get('type')
			self.w(f"\t{export_check_var(header_get_method_arg(arg), arg_type, i+idx)};")
			args.append(arg.get('name'))
		call = f"{header_get_method_name(node, method)}({', '.join(args)})"
		returns = method.find('returns')
		if returns is None:
			self.w(f"\t{call};\n\treturn 0;")
		elif returns.get('type') == 'nresults':
			self.w(f"\treturn {call};")
		else:
			ret = header_get_arg_type(returns)
			self.w(f"\t{ret} output = {call};\n\t{export_push_var(returns, 'output')};\n\treturn 1;")
		self.w(f"}}")

	def on_interface(self, root, interface):
		self.on_struct(root, interface)

	def on_struct(self, root, struct):
		is_struct = struct.tag == 'struct'
		name = struct.get('name')
		if is_struct:
			# write lua push
			self.w(f"void luaX_push{name}(lua_State *L, {lpc(name)} data) {{")
			self.w(f"\t{lp(name)} self = lua_newuserdata(L, sizeof(struct {name}));")
			self.w(f"\tluaL_setmetatable(L, \"{name}\");")
			self.w(f"\tmemcpy(self, data, sizeof(struct {name}));")
			self.w(f"}}")
		# write lua check
		if not struct.get("no-check"):
			self.w(f"{lp(name)} luaX_check{name}(lua_State *L, int idx) {{")
			self.w(f"\treturn luaL_checkudata(L, idx, \"{name}\");")
			self.w(f"}}")
		else:
			self.w(f"{lp(name)} luaX_check{name}(lua_State *L, int idx);")
		if is_struct:
			self.w(f"static int f_new_{name}(lua_State *L) {{")
			self.w(f"\t{lp(name)} self = lua_newuserdata(L, sizeof(struct {name}));")
			self.w(f"\tluaL_setmetatable(L, \"{name}\");")
			self.w(f"\tmemset(self, 0, sizeof(struct {name}));")
			for i, arg in enumerate(struct.findall('field')):
				arg_type = arg.get('type')
				if arg.get('array'):
					continue
				if arg_type not in atomic_types:
					continue
				a = f"self->{arg.get('name')}"
				self.w(f"\t{export_check_var(a, arg_type, i+1)};")
			self.w(f"\t// if (lua_istable(L, 1)) {{")
			self.w(f"\t// }}")
			self.w(f"\treturn 1;\n}}")
			self.w(f"static int f_{name}___call(lua_State *L) {{")
			self.w(f"\tlua_remove(L, 1); // remove {name} from stack")
			self.w(f"\treturn f_new_{name}(L);")
			self.w(f"}}")
		elif struct.find('init') is not None:
			self.w(f"int f_new_{name}(lua_State *L);")
		# write methods
		for method in struct.findall('method'):
			self.on_function(struct, method)
		# write __index function
		self.w(f"int {export_index_name(struct)}(lua_State *L) {{")
		if is_struct:
			self.w(f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
			for field in struct.findall('field'):
				if field.get('array') or field.get('private'): continue
				field_name = field.get('name')
				self.w(f"\tcase {hash(field_name)}: // {field_name}")
				self.w(f"\t\t{export_push_var(field, f'luaX_check{name}(L, 1)->{field_name}')};")
				self.w(f"\t\treturn 1;")
		else:
			self.w(f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
		for method in struct.findall('method'):
			method_name = method.get('export') or method.get('name')
			if method_name.startswith("__") or method.get('static') or method.get('private'):
				continue
			self.w(f"\tcase {hash(camel_case(method_name))}: // {camel_case(method_name)}")
			self.w(f"\t\tlua_pushcfunction(L, {export_get_name(struct, method)});")
			self.w(f"\t\treturn 1;")
		if is_struct:
			self.w(f"\t}}\n\treturn luaL_error(L, \"Unknown field in {name}: %s\", luaL_checkstring(L, 2));\n}}")
		else:
			self.w(f"\t}}\n\treturn 0;\n}}")
		if is_struct:
			# write __newindex function
			self.w(f"int {export_newindex_name(struct)}(lua_State *L) {{")
			self.w(f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
			for field in struct.findall('field'):
				if field.get('array'): continue
				field_name, field_type = field.get('name'), field.get('type')
				access = "*"
				if field_type in atomic_types: access = "" 
				if field_type in g_enums: access = ""
				if field_type in g_resources: access = ""
				if field_type in g_components: access = ""
				self.w(f"\tcase {hash(field_name)}: // {field_name}")
				self.w(f"\t\t{export_check_var(f'luaX_check{name}(L, 1)->{field_name}', field_type, 3, access)};")
				self.w(f"\t\treturn 0;")
			self.w(f"\t}}\n\treturn luaL_error(L, \"Unknown field in {name}: %s\", luaL_checkstring(L, 2));\n}}")
		# write lua_open
		self.w(f"int luaopen_{root.get('namespace')}_{name}(lua_State *L) {{") 
		self.w(f"\tluaL_newmetatable(L, \"{name}\");")
		self.w(f"\tluaL_setfuncs(L, ((luaL_Reg[]) {{")
		if is_struct or struct.find('init') is not None:
			self.w(f"\t\t{{ \"new\", f_new_{name} }},")
		if is_struct:
			self.w(f"\t\t{{ \"__newindex\", {export_newindex_name(struct)} }},")
		self.w(f"\t\t{{ \"__index\", {export_index_name(struct)} }},")
		for method in struct.findall('method'):
			method_name = method.get('export', method.get('name'))
			if not method_name.startswith("__") and not method.get('static'):
				continue
			self.w(f"\t\t{{ \"{camel_case(method_name)}\", {export_get_name(struct, method)} }},")
		self.w(f"\t\t{{ NULL, NULL }},")
		self.w(f"\t}}), 0);\n")

		if is_struct:# or struct.find('init') is not None:
			self.w(f"\tlua_newtable(L);")
			self.w(f"\tlua_pushcfunction(L, f_{name}___call);")
			self.w(f"\tlua_setfield(L, -2, \"__call\");")
			self.w(f"\tlua_setmetatable(L, -2);\n")

		self.w(f"\treturn 1;\n}}")

	def write_property(self, property, component, path):
		cname = component.get('name')
		if property.tag == 'shorthand':
			pname, userdata = property.get('name'), property.get('userdata')
			self.w(f"\t/* {cname}.{pname} */ DECL({hash(pname)}, {hash(cname+'.'+pname)},\n\t{cname}, \"{pname}\", {path}, kDataTypeEdges, .TypeString=\"{userdata}\"),")
		else:
			name = property_name(path)
			# self.w(f"#define ID_{cname}_{name} {hash(cname+'.'+name)}")
			sname = property_name(path)
			ptype = property.get('type')
			struct = g_structs.get(ptype)
			enum = None
			typedata = f"kDataType{ptype[:1].upper()+ptype[1:]}"
			if struct is not None:
				export_as = struct.get('export')
				nfields = 0
				def count(p, _, c): nonlocal nfields; nfields+=1
				enum_component_properties(struct, count)
				typedata = f"kDataType{export_as}" if export_as else f"kDataTypeGroup, .TypeString=\"{ptype}\", .NumComponents={nfields}"
			elif ptype in g_enums or sname in g_enums:
				enum = g_enums.get(ptype) or g_enums.get(sname)
				values = [e.get('name') for e in enum.findall('enum')]
				ptype = enum.get('name')
				typedata = f"kDataTypeEnum, .TypeString=\"{','.join(values)}\""
			# elif ptype in g_resources:
			# 	typedata = f"T_HANDLE, .TypeString=\"{ptype.capitalize()}\""
			# 	decor = 'resource'
			elif ptype in g_components:
				typedata = f"kDataTypeObject, .TypeString=\"{ptype}\""
			self.w(f"\t/* {cname}.{sname} */ DECL({hash(sname)}, {hash(cname+'.'+sname)},\n\t{cname}, \"{sname}\", {path}, {typedata}),")

	def on_component(self, root, component):
		cname = component.get('name')

		for handles in component.findall('handles'):
			event = handles.get('event')
			self.w(f"LRESULT {cname}_{event}({lpobject_t}, {lp(cname)}, wParam_t, {event}EventPtr);")
		self.w(f"static struct PropertyDesc const {cname}Properties[k{cname}NumProperties] = {{")

		for shorthand in component.findall("shorthand"):
			pname, physical, userdata = shorthand.get('name'), shorthand.get('physical'), shorthand.get('userdata')
			self.w(f"\t/* {cname}.{cname} */ DECL({hash(pname)}, {hash(cname+'.'+pname)},\n\t{cname}, \"{pname}\", {physical}, T_EDGES, .TypeString=\"{userdata}\"),")
		
		enum_component_properties(component, self.write_property)

		self.w(f"}};")

		hname = "InitDefaults"
		if bool(component.findall("property[@default]")):
			self.w(f"static struct {cname} {cname}Defaults = {{")
			for property in component.findall("property[@default]"):
				pname, ptype, pdefault = property.get('name'), property.get('type'), property.get('default')
				if ptype in g_enums and not pdefault.lstrip('-').isdigit():
					self.w(f"\t.{pname} = k{ptype}{pdefault},")
				elif ptype in g_structs:
					self.w(f"\t.{pname} = (struct {ptype}) {{ {', '.join(pdefault.split())} }},")
				elif ptype == 'bool':
					self.w(f"\t.{pname} = {pdefault.upper()},")
				elif ptype == 'fixed':
					self.w(f"\t.{pname} = \"{pdefault}\",")
				else:
					self.w(f"\t.{pname} = {pdefault},")
			self.w(f"}};")
		elif bool(component.findall("property")):
			self.w(f"static struct {cname} {cname}Defaults = {{0}};")
		else:
			self.w(f"static struct {cname} {cname}Defaults = {{}};")
		self.w(f"LRESULT {cname}Proc({lpobject_t} object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {{")
		self.w(f"\tswitch (message) {{")
		for handles in component.findall('handles'):
			hname = handles.get('event')
			self.w(f"\t\tcase {hash(hname)}: // {hname}")
			self.w(f"\t\t\treturn {cname}_{hname}(object, cmp, wparm, lparm);")			
		self.w(f"}}\n\treturn FALSE;\n}}")
		self.w(f"void luaX_push{cname}(lua_State *L, {lpc(cname)} {cname}) {{")
		self.w(f"\tluaX_pushObject(L, CMP_GetObject({cname}));\n}}")	
		self.w(f"{lp(cname)} luaX_check{cname}(lua_State *L, int idx) {{")
		self.w(f"\treturn Get{cname}(luaX_checkObject(L, idx));\n}}")	
		parents = component_get_parents(component)
		for p in parents:
			self.w(f"extern struct ClassDesc _{p};")
		parents = ", ".join([f"&_{p}" for p in parents]+["NULL"])
		content = component.get('parent', cname)
		if component.get('name').endswith('Library'):
			content = component.get('name').replace('Library', '')
		self.w(f"ORCA_API struct ClassDesc _{cname} = {{")
		self.w(f"\t.ClassName = \"{cname}\",")
		self.w(f"\t.DefaultName = \"{component.get('default', cname)}\",")
		self.w(f"\t.ContentType = \"{component.get('content', content)}\",")
		self.w(f"\t.Xmlns = \"{component.findtext('xmlns')}\",")
		self.w(f"\t.ParentClasses = {{{parents}}},")
		self.w(f"\t.ClassID = ID_{cname},")
		self.w(f"\t.ClassSize = sizeof(struct {cname}),")
		self.w(f"\t.Properties = {cname}Properties,")
		self.w(f"\t.ObjProc = {cname}Proc,")
		self.w(f"\t.Defaults = &{cname}Defaults,")
		self.w(f"\t.NumProperties = k{cname}NumProperties,\n}};");


def tokenize(code):
	token_re = re.compile(
		r'\s*"[^"\n]*"'   # double-quoted strings (no newlines inside)
		r'|--[^\n]*'      # single-line comments
		r"|\n"            # newline
		r"|\t"            # tab
		r"|\w+"           # identifiers/keywords
		r"|\d+"           # numbers
		r"|[^\w\d\n\t]"   # everything else (punctuation, operators)
	)
	for match in token_re.finditer(code):
		yield match.group(0)

rules = {
	"lua": "require|and|break|do|else|elseif|end|false|for|function|goto|if|in|local|nil|not|or|repeat|return|then|true|until|while"
}

def lpc(name): return f'lpc{name}_t'
def lp(name): return f'lp{name}_t'
def _c(name): return f'c{name}_t'
def _t(name): return f'{name}_t'
def _e(name): return f'e{name}_t'

lpobject_t = "lpObject_t"

def wrap_backticks(elem):
    xml_str = ET.tostring(elem, encoding="unicode")
    parts = xml_str.split("`")
    rebuilt = []
    for i, part in enumerate(parts):
        if i % 2 == 0:
            rebuilt.append(part)
        else:
            rebuilt.append(f"<code class=\"token-keyword\">{part}</code>")
    xml_str = "".join(rebuilt)
    return ET.fromstring(xml_str)

class doc:
	def add_header(div, type, name, brief):
		ET.SubElement(div, 'div', {'class':'eyebrow'}).text = type+"\n"
		ET.SubElement(div, 'h1', {'class':'header'}).text = name+"\n"
		if brief:
			ET.SubElement(div, 'div', {'class':'abstract'}).text = brief+"\n"

	def add_code(div, headers, type, name, parents):
		code = ET.SubElement(div, 'code', {'class':'source'})

		for key, value in headers:
			top = ET.SubElement(code, "div", {'class':'token-keyword'})
			ET.SubElement(top, 'span', {'class':'token-keyword'}).text = key
			ET.SubElement(top, 'span', {'class':'token-string'}).text = value

		ET.SubElement(code, "span", {'class':'token-keyword'}).text = type
		ET.SubElement(code, "span", {'class':'token-identifier'}).text = name

		if not parents:
			return

		ET.SubElement(code, 'span', {'class':'token-keyword'}).text = " inherits "
		cont = ET.SubElement(code, 'ui', {'class':'comma-list'})
		for n in parents.split(","):
			li = ET.SubElement(cont, 'li')
			ET.SubElement(li, 'a', {'href':'#'+n.strip(), 'class':'name base'}).text = n

	def add_overview(div, overview):
		if overview is None:
			return
		overview = wrap_backticks(overview)
		ET.SubElement(div, 'h2', {'class':'title'}).text = 'Overview'
		
		overview.tag = "div"
		overview.attrib['class'] = 'abstract'
		div.append(overview)

		for node in overview.findall('.//snippet[@src]'):
			with open(node.get('src')) as f:
				node.append(doc.highlight(f.read(), "lua"))
			node.tag = 'div'

	def print_abstract(div, doc):
		for i, n in enumerate(doc.split('`')):
			if i % 2:
				ET.SubElement(div, 'code', {'class':'token-keyword'}).text = n
			else:
				ET.SubElement(div, 'span').text = n

	def highlight(code, lang="lua"):
		root = ET.Element("code", {"class": "source code-listing"})
		kws = rules.get(lang, set()).split('|')
		for tok in tokenize(code):
			if tok in kws:
				ET.SubElement(root, "span", {"class": "syntax-keyword"}).text = tok
			elif '--' in tok:
				ET.SubElement(root, "span", {"class": "syntax-comment"}).text = tok
			elif '"' in tok:
				if root[-1].attrib['class'] == "syntax-variable":
					root[-1].attrib['class'] = "syntax-function"
				ET.SubElement(root, "span", {"class": "syntax-string"}).text = tok
			elif tok.isdigit():
				ET.SubElement(root, "span", {"class": "syntax-number"}).text = tok
			elif tok.isalnum():
				if tok[0].isupper():  # PascalCase
					ET.SubElement(root, "span", {"class": "syntax-type"}).text = tok
				else:
					ET.SubElement(root, "span", {"class": "syntax-variable"}).text = tok
			elif tok == '\n':
				ET.SubElement(root, "br")
			elif tok == '\t':
				ET.SubElement(root, "span").text = "\u00A0"*4
			elif root is not None:
				if ('{' in tok or '(' in tok) and root[-1].attrib['class'] in ["syntax-variable", "syntax-identifier"]:
					root[-1].attrib['class'] = "syntax-function"
				ET.SubElement(root, "span", {"class": "syntax-identifier"}).text = tok
		return root
	
	def add_small_abstract(topic, abstract):
		if not abstract:
			return
		doc.print_abstract(ET.SubElement(topic, 'div', {'class':'small-abstract'}), abstract+'\n')

	def add_property(topic, decor, name, type):
		code = ET.SubElement(topic, 'code', {'class':'decorator'})
		ET.SubElement(code, 'span').text = decor + " "
		nm = ET.SubElement(code, 'span', {'class':'identifier'})
		nm.text = name
		if type:
			nm.tail = ': '
			if type in g_enums or type in g_structs or type in g_components:
				ET.SubElement(code, 'a', {'class': 'type-identifier-link', 'href': '#'+type}).text = type
			else:
				ET.SubElement(code, 'span').text = type

	def add_article(body, name, parent, namespace):
		return ET.SubElement(body, 'div', {
			"id":name,
			'class':'class', 
			'data-name':name, 
			'data-inherits':parent, 
			'data-xmlns':namespace
		})

	def add_function(topic, function, returns):
		name = camel_case(function.get('name'))
		code = ET.SubElement(topic, 'code', {'class':'decorator'})
		ET.SubElement(code, 'span').text = 'func '
		nm = ET.SubElement(code, 'span', {'class':'identifier'})
		nm.text = name
		params = function.findall('arg')
		if params:
			nm.tail = '('
			while params:
				arg = params.pop(0)
				aname = ET.SubElement(code, 'span', {'class':'identifier'})
				aname.text = arg.get('name')
				aname.tail = ':'
				atype = ET.SubElement(code, 'span', {'class':'decorator'})
				atype.text = arg.get('type')
				atype.tail = params and ', ' or ')'
		else:
			nm.tail = '()'
		if returns is not None:
			ET.SubElement(code, 'span', {'class':'decorator'}).text = ' ->'
			ET.SubElement(code, 'span').text = " " + returns.get('type')

def fnv1a32(str):
	hval = 0x811C9DC5
	prime = 0x01000193
	uint32_max = 2 ** 32
	for s in str:
		hval = hval ^ ord(s)
		hval = (hval * prime) % uint32_max
	return hval

def hash(field_name): 
	return '0x%08x' % fnv1a32(field_name)

def w(f, text):
	if f:
		f.write(text)
		f.write("\n")

def wf(f, text, args, **kwargs):
	if args is None:
		args = {}
	else:
		args = dict(args.attrib)
	args.update(kwargs)
	f.write(text.format_map(args))
	f.write("\n")

atomic_types = { 
	"float": ("luaL_checknumber", "lua_pushnumber"),
	"int": ("luaL_checknumber", "lua_pushnumber"),
	"uint": ("luaL_checknumber", "lua_pushnumber"),
	"long": ("luaL_checkinteger", "lua_pushinteger"),
	"bool": ("lua_toboolean", "lua_pushboolean"),
	"string": ("luaL_checkstring", "lua_pushstring"),
	"fixed": ("luaL_checkstring", "lua_pushstring"),
	"handle": ("lua_touserdata", "lua_pushlightuserdata")
}

Axis = [
	(re.compile(r"(.+)\.Axis\[0\]\.Left(.*)"), r"\1Left\2"),
	(re.compile(r"(.+)\.Axis\[0\]\.Right(.*)"), r"\1Right\2"),
	(re.compile(r"(.+)\.Axis\[1\]\.Left(.*)"), r"\1Top\2"),
	(re.compile(r"(.+)\.Axis\[1\]\.Right(.*)"), r"\1Bottom\2"),
	(re.compile(r"(.+)\.Axis\[2\]\.Left(.*)"), r"\1Front\2"),
	(re.compile(r"(.+)\.Axis\[2\]\.Right(.*)"), r"\1Back\2"),
	(re.compile(r"Size.Axis\[0\]\.(?:Requested)?(.*)"), r"\1Width"),
	(re.compile(r"Size.Axis\[1\]\.(?:Requested)?(.*)"), r"\1Height"),
	(re.compile(r"Size.Axis\[2\]\.(?:Requested)?(.*)"), r"\1Depth"),
	# (re.compile(r"Size.Axis\[0\]\.(.*)"), r"\1Width"),
	# (re.compile(r"Size.Axis\[1\]\.(.*)"), r"\1Height"),
	# (re.compile(r"Size.Axis\[2\]\.(.*)"), r"\1Depth"),
	(re.compile(r"(.+)\.Axis\[0\]"), r"Horizontal\1"),
	(re.compile(r"(.+)\.Axis\[1\]"), r"Vertical\1"),
	(re.compile(r"(.+)\.Axis\[2\]"), r"Depth\1"),
	(re.compile(r"Border\.Radius\.(.+)Radius"), r"Border\1Radius"),
]

def throw_no_parser(root, node, _):
	pass
	# print("no such parser: ", node.tag)
	# exit(0)

def header_get_method_this(node, method):
	if method.get('const'):
		return f"{lpc(node.get('name'))} self"
	else:
		return f"{lp(node.get('name'))} self"

def header_get_method_name(node, method):
	return f"{node.get('prefix', node.get('name') + '_')}{method.get('name')}"

typedefs = {
	"nresults": "int32_t",
	"string": "const char*",
	"handle": "void*",
	"bool": "bool_t",
	"uint": "uint32_t",
	"long": "long",
	"int": "int32_t",
	"float": "float",
	"fixed": "fixedString_t",
}

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

def camel_case(variable):
	return variable[0].lower() + variable[1:]

def property_name(name):
	def conv_name(name):
		for pat, repl in Axis:
			s2, n = pat.subn(repl, name, count=1)
			if n: return s2
		return name
	name = conv_name(name)
	return "".join([s[0].upper() + s[1:] for s in name.split('.')])

def component_get_parents(component):
	parents = []
	if component.get('parent'):
		parents.append(component.get('parent'))
	if component.get('concept'):
		parents.append(component.get('concept'))
	return parents

def struct_property_cb(cmp, struct, prefix, func, *args):
	for shorthand in struct.findall("shorthand"):
		func(shorthand, cmp, shorthand.get('physical'), *args)
	for field in struct.findall('field') if not struct.get('sealed') else []:
		if field.get('array'):
			for m in range(int(field.get('array'))):
				name = '%s[%d]'%(field.get('name'), m)
				component_property_cb(cmp, prefix+name, field, func, *args)
		else:
			component_property_cb(cmp, prefix+field.get('name'), field, func, *args)
	
def component_property_cb(cmp, path, property, func, *args):
	if not property.get('exclude-self'):
		func(property, cmp, path, *args)
	struct = g_structs.get(property.get('type'))
	if struct is not None:
		struct_property_cb(cmp, struct, f"{path}.", func, *args)

def enum_component_properties(component, func, *args):
	if component.get('sealed'): 
		return
	for shorthand in component.findall("shorthand"):
		func(shorthand, component, shorthand.get('physical'), *args)
	for property in component.findall("property" if component.tag == 'component' else "field"):
		pname = property.get('name')
		if property.get('array'):
			for n in range(int(property.get('array'))):
				component_property_cb(component, '%s[%d]'%(pname,n), property, func, *args)
		else:
			component_property_cb(component, pname, property, func, *args)

writers = [
	DTDWriter(),
	HeaderWriter(),
	HtmlWriter(),
	PropertiesWriter(),
	ExportWriter(),
]

def call_plugin_hook(hook, *args):
	for writer in writers:
		method = getattr(writer, hook, None)
		if method:
			method(*args)

def read_xml(filename):
	if not os.path.exists(filename):
		print(f"File not found: {filename}")
		return

	tree = ET.parse(filename)
	root = tree.getroot()

	if root.tag != "module":
		print("Expected 'module' tag")
		return

	call_plugin_hook('open', filename, root)

	g_structs.update({struct.get('name'): struct for struct in root.findall(f".//struct[@name]")})
	g_enums.update({enum.get('name'): enum for enum in root.findall(f".//enums[@name]")})
	g_components.update({cmp.get('name'): cmp for cmp in root.findall(f".//component[@name]")})
	g_resources.update({cmp.get('type'): cmp for cmp in root.findall(f".//resource[@type]")})

	for struct in root.findall('struct')+root.findall('interface')+root.findall('component'):
		struct_name = struct.get('name')
		call_plugin_hook("struct_fwd_def", struct_name)
		call_plugin_hook("lua_accessors", struct_name)

	for node in root:
		call_plugin_hook(f"on_{node.tag}", root, node)

	call_plugin_hook('close')
  
if __name__ == "__main__":
	for i in range(len(sys.argv)-1):
		read_xml(sys.argv[i+1])

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

	call_plugin_hook('finalize')

