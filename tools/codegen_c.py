import re
import io

def lpc(name): return f'lpc{name}_t'
def lp(name): return f'lp{name}_t'
def _c(name): return f'c{name}_t'
def _t(name): return f'{name}_t'
def _e(name): return f'e{name}_t'

lpobject_t = "lpObject_t"

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
	(re.compile(r"(.+)\.Axis\[0\]"), r"Horizontal\1"),
	(re.compile(r"(.+)\.Axis\[1\]"), r"Vertical\1"),
	(re.compile(r"(.+)\.Axis\[2\]"), r"Depth\1"),
	(re.compile(r"Border\.Radius\.(.+)Radius"), r"Border\1Radius"),
]

class ParserState:
	def __new__(self, filename):
		self.header = open(filename.replace(".xml", ".h"), "w")
		self.export = open(filename.replace(".xml", "_export.c"), "w")
		self.props = open(filename.replace(".xml", "_properties.h"), "w")
		
		return self

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

def camel_case(variable):
	return variable[0].lower() + variable[1:]

def header_get_method_this(node, method):
	if method.get('const'):
		return f"{lpc(node.get('name'))} self"
	else:
		return f"{lp(node.get('name'))} self"

def header_get_method_name(node, method):
	return f"{node.get('prefix', node.get('name') + '_')}{method.get('name')}"

def header_get_arg_type(arg, registries):
	t = arg.get('type')
	if t in typedefs:
		return f"{typedefs[t]}*" if arg.get('pointer') else typedefs[t]
	if t in registries['enums']:
		return _e(t)
	if arg.get('pointer'):
		return lpc(t) if arg.get('const') else lp(t)
	else:
		return _c(t) if arg.get('const') else _t(t)

def header_get_method_arg(arg, registries):
	return f"{header_get_arg_type(arg, registries)} {arg.get('name')}"

def header_write_method(node, method, file, ctx):
	from doc_gen import doc
	args = []
	if method.get("lua"):
		args.append("lua_State *L")
	if method.tag == "method" and method.get('static') is None:
		args.append(header_get_method_this(node, method))
	if method.findtext("summary"):
		w(file, f"/// @brief {method.findtext('summary')}")
	for arg in method.findall('arg'):
		args.append(header_get_method_arg(arg, {'enums': ctx.enums}))
	args = ', '.join(args)
	returns = method.find('returns')
	ret = 'void' if returns is None else header_get_arg_type(returns, {'enums': ctx.enums})
	w(file, f"ORCA_API {ret}\n{header_get_method_name(node, method)}({args or 'void'});\n")

	topic = ET.SubElement(ctx.article, 'div', {'class':'topic'})
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

def export_push_var(ret, var, registries):
	rtype = ret.get('type')
	if rtype in atomic_types:
		_, push = atomic_types[rtype]
		return f"{push}(L, {var})"
	elif ret.get('pointer') or rtype in registries['enums']:
		return f"luaX_push{rtype}(L, {var})"
	else:
		return f"luaX_push{rtype}(L, &{var})"

def export_write_method(node, method, file, registries):
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
		w(file, f"\t{export_check_var(header_get_method_arg(arg, registries), arg_type, i+idx)};")
		args.append(arg.get('name'))
	call = f"{header_get_method_name(node, method)}({', '.join(args)})"
	returns = method.find('returns')
	if returns is None:
		w(file, f"\t{call};\n\treturn 0;")
	elif returns.get('type') == 'nresults':
		w(file, f"\treturn {call};")
	else:
		ret = header_get_arg_type(returns, registries)
		w(file, f"\t{ret} output = {call};\n\t{export_push_var(returns, 'output', registries)};\n\treturn 1;")
	w(file, f"}}")

def write_struct_fwd_def(sname, typedef, parser):
	w(parser, f"typedef struct {sname} {_t(typedef)}, *{lp(typedef)};")
	w(parser, f"typedef struct {sname} const {_c(typedef)}, *{lpc(typedef)};")

def write_push_check(name, h):
	w(h, f"/// @brief Push {name} onto Lua stack.")
	w(h, f"ORCA_API void\nluaX_push{name}(lua_State *L, {lpc(name)} {name});")
	w(h, f"/// @brief Check {name} form Lua stack at index.")
	w(h, f"ORCA_API {lp(name)}\nluaX_check{name}(lua_State *L, int idx);\n")

# Needed for header_write_method
import xml.etree.ElementTree as ET
