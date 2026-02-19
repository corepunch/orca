"""
Export Plugin - Generates C export/Lua binding files (_export.c)
"""

from . import OutputPlugin
from .common import w, lpc, lp, _c, _t, _e, typedefs, atomic_types, camel_case

class ExportPlugin(OutputPlugin):
	"""Plugin for generating C export/Lua binding files (_export.c)"""
	def __init__(self, filename):
		super().__init__(filename, "_export.c")
		self.g_structs = {}
		self.g_enums = {}
		self.header_plugin = None
	
	def set_globals(self, structs, enums, header_plugin=None):
		"""Set global references needed for generation"""
		self.g_structs = structs
		self.g_enums = enums
		self.header_plugin = header_plugin
	
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

