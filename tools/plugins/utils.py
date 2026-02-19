"""Shared utility functions and data used by multiple plugins."""

import re
import xml.etree.ElementTree as ET

from .state import g_structs, g_enums, g_components, g_resources

# ---------------------------------------------------------------------------
# Type helpers
# ---------------------------------------------------------------------------

def lpc(name): return f'lpc{name}_t'
def lp(name):  return f'lp{name}_t'
def _c(name):  return f'c{name}_t'
def _t(name):  return f'{name}_t'
def _e(name):  return f'e{name}_t'

lpobject_t = "lpObject_t"

typedefs = {
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

# ---------------------------------------------------------------------------
# Hash
# ---------------------------------------------------------------------------

def fnv1a32(s):
	hval = 0x811C9DC5
	prime = 0x01000193
	uint32_max = 2 ** 32
	for ch in s:
		hval = hval ^ ord(ch)
		hval = (hval * prime) % uint32_max
	return hval

def hash(field_name):
	return '0x%08x' % fnv1a32(field_name)

# ---------------------------------------------------------------------------
# Naming helpers
# ---------------------------------------------------------------------------

def camel_case(variable):
	return variable[0].lower() + variable[1:]

#: Axis substitution rules used by property_name().
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

def property_name(name):
	def conv_name(name):
		for pat, repl in Axis:
			s2, n = pat.subn(repl, name, count=1)
			if n:
				return s2
		return name
	name = conv_name(name)
	return "".join([s[0].upper() + s[1:] for s in name.split('.')])

# ---------------------------------------------------------------------------
# Header-generation helpers
# ---------------------------------------------------------------------------

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

# ---------------------------------------------------------------------------
# Export-generation helpers
# ---------------------------------------------------------------------------

def export_get_name(node, method):
	return f"f_{node.get('name')}_{camel_case(method.get('export') or method.get('name'))}"

def export_index_name(node):
	return f"f_{node.get('name')}___index"

def export_newindex_name(node):
	return f"f_{node.get('name')}___newindex"

def export_check_var(target, arg_type, idx, access=''):
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

# ---------------------------------------------------------------------------
# Component/struct property enumeration helpers
# ---------------------------------------------------------------------------

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
				name = '%s[%d]' % (field.get('name'), m)
				component_property_cb(cmp, prefix + name, field, func, *args)
		else:
			component_property_cb(cmp, prefix + field.get('name'), field, func, *args)

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
	for property in component.findall(
		"property" if component.tag == 'component' else "field"
	):
		pname = property.get('name')
		if property.get('array'):
			for n in range(int(property.get('array'))):
				component_property_cb(component, '%s[%d]' % (pname, n), property, func, *args)
		else:
			component_property_cb(component, pname, property, func, *args)

# ---------------------------------------------------------------------------
# HTML / documentation helpers
# ---------------------------------------------------------------------------

#: Tokeniser rules for syntax highlighting.
_token_rules = {
	"lua": (
		"require|and|break|do|else|elseif|end|false|for|function|goto|if|"
		"in|local|nil|not|or|repeat|return|then|true|until|while"
	)
}

_token_re = re.compile(
	r'\s*"[^"\n]*"'   # double-quoted strings (no newlines)
	r'|--[^\n]*'      # single-line comments
	r"|\n"            # newline
	r"|\t"            # tab
	r"|\w+"           # identifiers / keywords
	r"|\d+"           # numbers
	r"|[^\w\d\n\t]"   # everything else
)

def tokenize(code):
	for match in _token_re.finditer(code):
		yield match.group(0)

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
