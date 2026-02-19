"""
Header Plugin - Generates C header files (.h)
"""

from . import OutputPlugin
from .common import w, lpc, lp, _c, _t, _e, typedefs

class HeaderPlugin(OutputPlugin):
	"""Plugin for generating C header files (.h)"""
	def __init__(self, filename):
		super().__init__(filename, ".h")
		self.g_structs = {}
		self.g_enums = {}
	
	def set_globals(self, structs, enums):
		"""Set global references needed for generation"""
		self.g_structs = structs
		self.g_enums = enums
	
	def get_method_this(self, node, method):
		"""Get the 'this' parameter for a method"""
		if method.get('const'):
			return f"{lpc(node.get('name'))} self"
		else:
			return f"{lp(node.get('name'))} self"
	
	def get_method_name(self, node, method):
		"""Get the full method name"""
		return f"{node.get('prefix', node.get('name') + '_')}{method.get('name')}"
	
	def get_arg_type(self, arg):
		"""Get the C type for an argument"""
		t = arg.get('type')
		if t in typedefs:
			return f"{typedefs[t]}*" if arg.get('pointer') else typedefs[t]
		if t in self.g_enums:
			return _e(t)
		if arg.get('pointer'):
			return lpc(t) if arg.get('const') else lp(t)
		else:
			return _c(t) if arg.get('const') else _t(t)
	
	def get_method_arg(self, arg):
		"""Format a method argument"""
		return f"{self.get_arg_type(arg)} {arg.get('name')}"
	
	def write_method(self, node, method, doc_callback=None):
		"""Write a method declaration"""
		args = []
		if method.get("lua"):
			args.append("lua_State *L")
		if method.tag == "method" and method.get('static') is None:
			args.append(self.get_method_this(node, method))
		
		if method.findtext("summary"):
			w(self.file_handle, f"/// @brief {method.findtext('summary')}")
		
		for arg in method.findall('arg'):
			args.append(self.get_method_arg(arg))
		args = ', '.join(args)
		returns = method.find('returns')
		ret = 'void' if returns is None else self.get_arg_type(returns)
		w(self.file_handle, f"ORCA_API {ret}\n{self.get_method_name(node, method)}({args or 'void'});\n")
		
		# Call documentation callback if provided
		if doc_callback:
			doc_callback(method, returns and returns.get('type'))
	
	def write_struct_fwd_def(self, sname, typedef):
		"""Write forward struct definition"""
		w(self.file_handle, f"typedef struct {sname} {typedef}_t, *lp{typedef}_t;")
		w(self.file_handle, f"typedef struct {sname} const c{typedef}_t, *lpc{typedef}_t;")
	
	def write_push_check(self, name):
		"""Write push/check declarations"""
		w(self.file_handle, f"/// @brief Push {name} onto Lua stack.")
		w(self.file_handle, f"ORCA_API void\nluaX_push{name}(lua_State *L, lpc{name}_t {name});")
		w(self.file_handle, f"/// @brief Check {name} form Lua stack at index.")
		w(self.file_handle, f"ORCA_API lp{name}_t\nluaX_check{name}(lua_State *L, int idx);\n")
	
	def write_struct_def(self, root, struct):
		"""Write struct definition"""
		sname = struct.get('name')
		w(self.file_handle, f"struct {sname} {{")
		
		parent = struct.get('inherits')
		if parent:
			w(self.file_handle, f"\tstruct {parent} base;")
		
		for field in struct.findall('field'):
			field_type = field.get('type')
			field_name = field.get('name')
			
			if field_type in typedefs:
				ctype = typedefs[field_type]
			elif field_type in self.g_enums:
				ctype = _e(field_type)
			elif field.get('pointer'):
				ctype = lp(field_type)
			else:
				ctype = _t(field_type)
			
			if field.get('array'):
				w(self.file_handle, f"\t{ctype} {field_name}[{field.get('array')}];")
			else:
				w(self.file_handle, f"\t{ctype} {field_name};")
		
		w(self.file_handle, f"}};")
		w(self.file_handle, "")

