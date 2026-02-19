"""
Export Plugin - Generates C export/Lua binding files (_export.c)
"""

from . import OutputPlugin
from .common import w, lpc, lp, _c, _t, _e, typedefs, atomic_types, camel_case, hash, fnv1a32

class ExportPlugin(OutputPlugin):
	"""Plugin for generating C export/Lua binding files (_export.c)"""
	def __init__(self, filename):
		super().__init__(filename, "_export.c")
		self.g_structs = {}
		self.g_enums = {}
		self.g_components = {}
		self.g_resources = {}
		self.header_plugin = None
	
	def set_globals(self, structs, enums, header_plugin=None, components=None, resources=None):
		"""Set global references needed for generation"""
		self.g_structs = structs
		self.g_enums = enums
		self.header_plugin = header_plugin
		if components:
			self.g_components = components
		if resources:
			self.g_resources = resources
	
	def get_name(self, node, method):
		"""Get the export function name"""
		return f"f_{node.get('name')}_{camel_case(method.get('export') or method.get('name'))}"
	
	def get_index_name(self, node):
		"""Get the __index function name"""
		return f"f_{node.get('name')}___index"
	
	def get_newindex_name(self, node):
		"""Get the __newindex function name"""
		return f"f_{node.get('name')}___newindex"
	
	def check_var(self, target, arg_type, idx, access=''):
		"""Generate code to check/get a variable from Lua stack"""
		if arg_type == 'STRING':
			return f"strncpy({target}, luaL_checkstring(L, {idx}), sizeof(STRING));"
		elif arg_type in atomic_types:
			check, _ = atomic_types[arg_type]
			return f"{target} = {access}{check}(L, {idx})"
		else:
			return f"{target} = {access}luaX_check{arg_type}(L, {idx})"
	
	def push_var(self, ret, var):
		"""Generate code to push a variable onto Lua stack"""
		rtype = ret.get('type')
		if rtype in atomic_types:
			_, push = atomic_types[rtype]
			return f"{push}(L, {var})"
		elif ret.get('pointer') or rtype in self.g_enums:
			return f"luaX_push{rtype}(L, {var})"
		else:
			return f"luaX_push{rtype}(L, &{var})"
	
	def write_method(self, node, method):
		"""Write a method binding"""
		if method.get('private'):
			return
		
		args = []
		idx = 1
		if method.get("lua"):
			args.append("L")
		
		w(self.file_handle, f"static int {self.get_name(node, method)}(lua_State *L) {{")
		
		if method.tag == "method" and method.get('static') is None:
			# Use header plugin to get method types
			if self.header_plugin:
				method_this = self.header_plugin.get_method_this(node, method)
			else:
				# Fallback
				if method.get('const'):
					method_this = f"{lpc(node.get('name'))} self"
				else:
					method_this = f"{lp(node.get('name'))} self"
			
			w(self.file_handle, f"\t{method_this} = luaX_check{node.get('name')}(L, 1);")
			args.append("self")
			idx = 2
		
		for i, arg in enumerate(method.findall('arg')):
			arg_type = arg.get('type')
			if self.header_plugin:
				arg_decl = self.header_plugin.get_method_arg(arg)
			else:
				# Fallback
				arg_decl = f"{arg_type} {arg.get('name')}"
			
			w(self.file_handle, f"\t{self.check_var(arg_decl, arg_type, i+idx)};")
			args.append(arg.get('name'))
		
		# Get method name
		if self.header_plugin:
			method_name = self.header_plugin.get_method_name(node, method)
		else:
			method_name = f"{node.get('prefix', node.get('name') + '_')}{method.get('name')}"
		
		call = f"{method_name}({', '.join(args)})"
		returns = method.find('returns')
		
		if returns is None:
			w(self.file_handle, f"\t{call};\n\treturn 0;")
		elif returns.get('type') == 'nresults':
			w(self.file_handle, f"\treturn {call};")
		else:
			if self.header_plugin:
				ret = self.header_plugin.get_arg_type(returns)
			else:
				ret = returns.get('type')
			w(self.file_handle, f"\t{ret} output = {call};\n\t{self.push_var(returns, 'output')};\n\treturn 1;")
		
		w(self.file_handle, f"}}")
	
	def write_struct_bindings(self, root, struct):
		"""Write all Lua bindings for a struct"""
		is_struct = struct.tag == 'struct'
		name = struct.get('name')
		
		# Write lua push function
		if is_struct:
			w(self.file_handle, f"void luaX_push{name}(lua_State *L, {lpc(name)} data) {{")
			w(self.file_handle, f"\t{lp(name)} self = lua_newuserdata(L, sizeof(struct {name}));")
			w(self.file_handle, f"\tluaL_setmetatable(L, \"{name}\");")
			w(self.file_handle, f"\tmemcpy(self, data, sizeof(struct {name}));")
			w(self.file_handle, f"}}")
		
		# Write lua check function
		if not struct.get("no-check"):
			w(self.file_handle, f"{lp(name)} luaX_check{name}(lua_State *L, int idx) {{")
			w(self.file_handle, f"\treturn luaL_checkudata(L, idx, \"{name}\");")
			w(self.file_handle, f"}}")
		else:
			w(self.file_handle, f"{lp(name)} luaX_check{name}(lua_State *L, int idx);")
		
		# Write constructor function
		if is_struct:
			w(self.file_handle, f"static int f_new_{name}(lua_State *L) {{")
			w(self.file_handle, f"\t{lp(name)} self = lua_newuserdata(L, sizeof(struct {name}));")
			w(self.file_handle, f"\tluaL_setmetatable(L, \"{name}\");")
			w(self.file_handle, f"\tmemset(self, 0, sizeof(struct {name}));")
			for i, arg in enumerate(struct.findall('field')):
				arg_type = arg.get('type')
				if arg.get('array'):
					continue
				if arg_type not in atomic_types:
					continue
				a = f"self->{arg.get('name')}"
				w(self.file_handle, f"\t{self.check_var(a, arg_type, i+1)};")
			w(self.file_handle, f"\t// if (lua_istable(L, 1)) {{")
			w(self.file_handle, f"\t// }}")
			w(self.file_handle, f"\treturn 1;\n}}")
			w(self.file_handle, f"static int f_{name}___call(lua_State *L) {{")
			w(self.file_handle, f"\tlua_remove(L, 1); // remove {name} from stack")
			w(self.file_handle, f"\treturn f_new_{name}(L);")
			w(self.file_handle, f"}}")
		elif struct.find('init') is not None:
			w(self.file_handle, f"int f_new_{name}(lua_State *L);")
	
	def write_index_function(self, root, struct):
		"""Write the __index metamethod"""
		is_struct = struct.tag == 'struct'
		name = struct.get('name')
		
		w(self.file_handle, f"int {self.get_index_name(struct)}(lua_State *L) {{")
		if is_struct:
			w(self.file_handle, f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
			for field in struct.findall('field'):
				if field.get('array') or field.get('private'):
					continue
				field_name = field.get('name')
				w(self.file_handle, f"\tcase {hash(field_name)}: // {field_name}")
				w(self.file_handle, f"\t\t{self.push_var(field, f'luaX_check{name}(L, 1)->{field_name}')};")
				w(self.file_handle, f"\t\treturn 1;")
		else:
			w(self.file_handle, f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
		
		for method in struct.findall('method'):
			method_name = method.get('export') or method.get('name')
			if method_name.startswith("__") or method.get('static') or method.get('private'):
				continue
			w(self.file_handle, f"\tcase {hash(camel_case(method_name))}: // {camel_case(method_name)}")
			w(self.file_handle, f"\t\tlua_pushcfunction(L, {self.get_name(struct, method)});")
			w(self.file_handle, f"\t\treturn 1;")
		
		if is_struct:
			w(self.file_handle, f"\t}}\n\treturn luaL_error(L, \"Unknown field in {name}: %s\", luaL_checkstring(L, 2));\n}}")
		else:
			w(self.file_handle, f"\t}}\n\treturn 0;\n}}")
	
	def write_newindex_function(self, root, struct):
		"""Write the __newindex metamethod"""
		is_struct = struct.tag == 'struct'
		name = struct.get('name')
		
		if not is_struct:
			return
		
		w(self.file_handle, f"int {self.get_newindex_name(struct)}(lua_State *L) {{")
		w(self.file_handle, f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
		for field in struct.findall('field'):
			if field.get('array'):
				continue
			field_name, field_type = field.get('name'), field.get('type')
			access = "*"
			if field_type in atomic_types:
				access = ""
			if field_type in self.g_enums:
				access = ""
			if field_type in self.g_resources:
				access = ""
			if field_type in self.g_components:
				access = ""
			w(self.file_handle, f"\tcase {hash(field_name)}: // {field_name}")
			w(self.file_handle, f"\t\t{self.check_var(f'luaX_check{name}(L, 1)->{field_name}', field_type, 3, access)};")
			w(self.file_handle, f"\t\treturn 0;")
		w(self.file_handle, f"\t}}\n\treturn luaL_error(L, \"Unknown field in {name}: %s\", luaL_checkstring(L, 2));\n}}")
	
	def write_luaopen_function(self, root, struct):
		"""Write the luaopen function for the struct"""
		is_struct = struct.tag == 'struct'
		name = struct.get('name')
		namespace = root.get('namespace')
		
		w(self.file_handle, f"int luaopen_{namespace}_{name}(lua_State *L) {{")
		w(self.file_handle, f"\tluaL_newmetatable(L, \"{name}\");")
		w(self.file_handle, f"\tluaL_setfuncs(L, ((luaL_Reg[]) {{")
		if is_struct or struct.find('init') is not None:
			w(self.file_handle, f"\t\t{{ \"new\", f_new_{name} }},")
		if is_struct:
			w(self.file_handle, f"\t\t{{ \"__newindex\", {self.get_newindex_name(struct)} }},")
		w(self.file_handle, f"\t\t{{ \"__index\", {self.get_index_name(struct)} }},")
		for method in struct.findall('method'):
			method_name = method.get('export', method.get('name'))
			if not method_name.startswith("__") and not method.get('static'):
				continue
			w(self.file_handle, f"\t\t{{ \"{camel_case(method_name)}\", {self.get_name(struct, method)} }},")
		w(self.file_handle, f"\t\t{{ NULL, NULL }},")
		w(self.file_handle, f"\t}}), 0);\n")
		
		if is_struct:
			w(self.file_handle, f"\tlua_newtable(L);")
			w(self.file_handle, f"\tlua_pushcfunction(L, f_{name}___call);")
			w(self.file_handle, f"\tlua_setfield(L, -2, \"__call\");")
			w(self.file_handle, f"\tlua_setmetatable(L, -2);\n")
		
		w(self.file_handle, f"\treturn 1;\n}}")

