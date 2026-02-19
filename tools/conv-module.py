import sys, re, io, os
import xml.etree.ElementTree as ET

g_structs = {}
g_enums = {}
g_components = {}
g_resources = {}
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
g_output = "../docs/index.html"

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
		if struct:
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
			elif root:
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

class ParserState:
	def __new__(self, filename):
		self.export = open(filename.replace(".xml", "_export.c"), "w")
		self.props = open(filename.replace(".xml", "_properties.h"), "w")
		
		return self
	
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

def header_write_method(node, method):
	returns = method.find('returns')
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
	header_write_method(root, function)
	export_write_method(root, function, parser.export)

def write_struct_def(root, struct):
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

	for field in struct.findall('property')+struct.findall('field'):
		fname, ftype = field.get('name'), header_get_arg_type(field)
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

def camel_case(variable):
	return variable[0].lower() + variable[1:]

def struct_parse(root, struct, parser):
	e = parser.export
	is_struct = struct.tag == 'struct'
	name = struct.get('name')
	write_struct_def(root, struct)
	if is_struct:
		# write lua push
		w(e, f"void luaX_push{name}(lua_State *L, {lpc(name)} data) {{")
		w(e, f"\t{lp(name)} self = lua_newuserdata(L, sizeof(struct {name}));")
		w(e, f"\tluaL_setmetatable(L, \"{name}\");")
		w(e, f"\tmemcpy(self, data, sizeof(struct {name}));")
		w(e, f"}}")
	# write lua check
	if not struct.get("no-check"):
		w(e, f"{lp(name)} luaX_check{name}(lua_State *L, int idx) {{")
		w(e, f"\treturn luaL_checkudata(L, idx, \"{name}\");")
		w(e, f"}}")
	else:
		w(e, f"{lp(name)} luaX_check{name}(lua_State *L, int idx);")
	if is_struct:
		w(e, f"static int f_new_{name}(lua_State *L) {{")
		w(e, f"\t{lp(name)} self = lua_newuserdata(L, sizeof(struct {name}));")
		w(e, f"\tluaL_setmetatable(L, \"{name}\");")
		w(e, f"\tmemset(self, 0, sizeof(struct {name}));")
		for i, arg in enumerate(struct.findall('field')):
			arg_type = arg.get('type')
			if arg.get('array'):
				continue
			if arg_type not in atomic_types:
				continue
			a = f"self->{arg.get('name')}"
			w(e, f"\t{export_check_var(a, arg_type, i+1)};")
		w(e, f"\t// if (lua_istable(L, 1)) {{")
		w(e, f"\t// }}")
		w(e, f"\treturn 1;\n}}")
		w(e, f"static int f_{name}___call(lua_State *L) {{")
		w(e, f"\tlua_remove(L, 1); // remove {name} from stack")
		w(e, f"\treturn f_new_{name}(L);")
		w(e, f"}}")
	elif struct.find('init') is not None:
		w(e, f"int f_new_{name}(lua_State *L);")
	# write methods
	for method in struct.findall('method'):
		function_parse(struct, method, parser)
	# write __index function
	w(e, f"int {export_index_name(struct)}(lua_State *L) {{")
	if is_struct:
		w(e, f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
		for field in struct.findall('field'):
			if field.get('array') or field.get('private'): continue
			field_name = field.get('name')
			w(e, f"\tcase {hash(field_name)}: // {field_name}")
			w(e, f"\t\t{export_push_var(field, f'luaX_check{name}(L, 1)->{field_name}')};")
			w(e, f"\t\treturn 1;")
	else:
		w(e, f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
	for method in struct.findall('method'):
		method_name = method.get('export') or method.get('name')
		if method_name.startswith("__") or method.get('static') or method.get('private'):
			continue
		w(e, f"\tcase {hash(camel_case(method_name))}: // {camel_case(method_name)}")
		w(e, f"\t\tlua_pushcfunction(L, {export_get_name(struct, method)});")
		w(e, f"\t\treturn 1;")
	if is_struct:
		w(e, f"\t}}\n\treturn luaL_error(L, \"Unknown field in {name}: %s\", luaL_checkstring(L, 2));\n}}")
	else:
		w(e, f"\t}}\n\treturn 0;\n}}")
	if is_struct:
		# write __newindex function
		w(e, f"int {export_newindex_name(struct)}(lua_State *L) {{")
		w(e, f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
		for field in struct.findall('field'):
			if field.get('array'): continue
			field_name, field_type = field.get('name'), field.get('type')
			access = "*"
			if field_type in atomic_types: access = "" 
			if field_type in g_enums: access = ""
			if field_type in g_resources: access = ""
			if field_type in g_components: access = ""
			w(e, f"\tcase {hash(field_name)}: // {field_name}")
			w(e, f"\t\t{export_check_var(f'luaX_check{name}(L, 1)->{field_name}', field_type, 3, access)};")
			w(e, f"\t\treturn 0;")
		w(e, f"\t}}\n\treturn luaL_error(L, \"Unknown field in {name}: %s\", luaL_checkstring(L, 2));\n}}")
	# write lua_open
	w(e, f"int luaopen_{root.get('namespace')}_{name}(lua_State *L) {{") 
	w(e, f"\tluaL_newmetatable(L, \"{name}\");")
	w(e, f"\tluaL_setfuncs(L, ((luaL_Reg[]) {{")
	if is_struct or struct.find('init') is not None:
		w(e, f"\t\t{{ \"new\", f_new_{name} }},")
	if is_struct:
		w(e, f"\t\t{{ \"__newindex\", {export_newindex_name(struct)} }},")
	w(e, f"\t\t{{ \"__index\", {export_index_name(struct)} }},")
	for method in struct.findall('method'):
		method_name = method.get('export', method.get('name'))
		if not method_name.startswith("__") and not method.get('static'):
			continue
		w(e, f"\t\t{{ \"{camel_case(method_name)}\", {export_get_name(struct, method)} }},")
	w(e, f"\t\t{{ NULL, NULL }},")
	w(e, f"\t}}), 0);\n")

	if is_struct:# or struct.find('init') is not None:
		w(e, f"\tlua_newtable(L);")
		w(e, f"\tlua_pushcfunction(L, f_{name}___call);")
		w(e, f"\tlua_setfield(L, -2, \"__call\");")
		w(e, f"\tlua_setmetatable(L, -2);\n")

	w(e, f"\treturn 1;\n}}")

def enums_parse(_, enums, parser):
	e = parser.export
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

	for enum in enums.findall('enum'):
		topic = ET.SubElement(article, 'div', {'class':'topic'})
		topic.tail = "\n"
		doc.add_property(topic, 'case', enum.get('name'), '')
		doc.add_small_abstract(topic, enum.text)

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
			e, s = parser.export, parser.props
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
	e, s = parser.export, parser.props
	cname = component.get('name')

	write_struct_def(root, component)
	for method in component.findall('method'):
		header_write_method(component, method)
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

def resource_parse(_, resource, parser):
	if resource.get('no-lua'):
		return
	name = resource.get('type')
	w(parser.export, f"void luaX_push{name}(lua_State *L, {lpc(name)} {name}) {{")
	w(parser.export, f"\tlua_pushlightuserdata(L, ({lp(name)}){name});\n}}")	
	w(parser.export, f"{lp(name)} luaX_check{name}(lua_State *L, int idx) {{")
	w(parser.export, f"\treturn lua_touserdata(L, idx);\n}}")	

global_parsers={
	'struct': struct_parse,
	'interface': struct_parse,
	'function': function_parse,
	'enums': enums_parse,
	'component': component_parse,
	'resource': resource_parse,
}

writers = [
	DTDWriter(),
	HeaderWriter(),
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
	parser = ParserState(filename)
	subpath = filename[filename.index("source/"):]

	call_plugin_hook('open', filename, root)

	if root.tag != "module":
		print("Expected 'module' tag")
		return

	tech = ET.SubElement(g_sidebar, 'div', {'class':'technology-title'})
	i = ET.SubElement(tech, 'span', {'class':'fa-solid fa-folder-open icon'})
	i.tail = f"{root.get('namespace')}.{root.get('name')}\n"

	e = parser.export

	w(e, f"// Generated by codegen utility")
	w(e, f"#include <include/api.h>")
	w(e, f"#include <{subpath.replace('.xml', '.h')}>")

	if root.find('component') is not None:
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

	for struct in root.findall('struct')+root.findall('interface')+root.findall('component'):
		struct_name = struct.get('name')
		call_plugin_hook("struct_fwd_def", struct_name)
		call_plugin_hook("lua_accessors", struct_name)

	for node in root:
		global_parsers.get(node.tag, throw_no_parser)(root, node, parser)
		call_plugin_hook(f"on_{node.tag}", root, node)

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

	call_plugin_hook('close')
  
if __name__ == "__main__":
	for i in range(len(sys.argv)-1):
		read_xml(sys.argv[i+1])

	tree = ET.ElementTree(g_html)
	# ET.indent(tree, space="\t", level=0)
	tree.write(g_output, encoding="utf-8", method="html")

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

