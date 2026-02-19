"""
Common utilities for all plugins
"""

import re

# Type name generators
def lpc(name): return f'lpc{name}_t'
def lp(name): return f'lp{name}_t'
def _c(name): return f'c{name}_t'
def _t(name): return f'{name}_t'
def _e(name): return f'e{name}_t'

lpobject_t = "lpObject_t"

# Camel case converter
def camel_case(name):
	components = name.split('_')
	return components[0] + ''.join(x.title() for x in components[1:])

# Hash functions
def fnv1a32(str):
	hval = 0x811c9dc5
	prime = 0x01000193
	uint32_max = 2 ** 32
	for s in str:
		hval = hval ^ ord(s)
		hval = (hval * prime) % uint32_max
	return hval

def hash(field_name): 
	return '0x%08x' % fnv1a32(field_name)

# Write helper functions
def w(f, text):
	"""Write a line to file"""
	if f:
		f.write(text)
		f.write("\n")

def wf(f, text, args, **kwargs):
	"""Write formatted text to file"""
	if args is None:
		args = {}
	else:
		args = dict(args.attrib)
	args.update(kwargs)
	f.write(text.format_map(args))
	f.write("\n")

# Type definitions
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
