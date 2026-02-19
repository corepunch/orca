import xml.etree.ElementTree as ET
from codegen_c import (
    w, hash, lpc, lp, _t, _c, atomic_types, 
    header_get_arg_type, header_write_method,
    export_get_name, export_index_name, export_newindex_name,
    export_check_var, export_push_var, export_write_method,
    camel_case, write_struct_fwd_def
)
from doc_gen import doc

def write_struct_def(root, struct, h, ctx):
	sname, sbrief = struct.get('name'), struct.findtext('summary')

	ctx.article = doc.add_article(ctx.content, sname, struct.get('parent') or 'None', struct.findtext('xmlns') or 'None')
	atts = []
	doc.add_header(ctx.article, struct.tag.capitalize(), sname, sbrief)
	doc.add_code(ctx.article, atts, "struct ", "%s.%s"%(root.get('name'),sname), struct.get('parent') or (struct.tag == 'component' and 'object'))
	doc.add_overview(ctx.article, struct.find('details'))

	icons = {
		'struct': 'gear',
		'component': 'layer-group',
		'interface': 'atom',
	}

	link = ET.SubElement(ctx.sidebar, 'a', {'class':'leaf-link', 'href':f'#{sname}'})
	ET.SubElement(link, 'i', {'class':f'fa-solid fa-{icons.get(struct.tag)} icon'})
	ET.SubElement(link, 'span').text = sname+'\n'

	if struct.findall('property') or struct.findall('methods'):
		ET.SubElement(ctx.article, 'h2', {'class':'title'}).text = 'Properties'

	if struct.tag == "interface":
		return

	if sbrief: w(h, f"/// @brief {sbrief}")
	w(h, f"struct {sname} {{")
	registries = {'structs': ctx.structs, 'enums': ctx.enums, 'components': ctx.components, 'resources': ctx.resources}
	for field in struct.findall('property')+struct.findall('field'):
		fname, ftype = field.get('name'), header_get_arg_type(field, registries)
		fcomment = f" /// {field.text}" if field.text else str()
		if field.get('array'):
			w(h, f"\t{ftype} {fname}[{field.get('array')}];{fcomment}")
		else:
			w(h, f"\t{ftype} {fname};{fcomment}")
		decor = 'var'
		ftype = field.get('type')
		if ftype in ctx.enums:
			decor = 'enum'
		if ftype in ctx.components:
			decor = 'object'
		if ftype in ctx.resources:
			decor = 'resource'
		if field.tag == 'field':
			topic = ET.SubElement(ctx.article, 'div', {'class':'topic'})
			topic.tail = "\n"
			doc.add_property(topic, decor, fname, ftype, registries)
			doc.add_small_abstract(ctx.article, field.text)
	w(h, f"}};\n")

def function_parse(root, function, parser, ctx):
	registries = {'enums': ctx.enums}
	header_write_method(root, function, parser.header, ctx)
	export_write_method(root, function, parser.export, registries)

def parse(root, struct, parser, ctx):
	e = parser.export
	is_struct = struct.tag == 'struct'
	name = struct.get('name')
	registries = {'structs': ctx.structs, 'enums': ctx.enums, 'components': ctx.components, 'resources': ctx.resources}
	
	write_struct_def(root, struct, parser.header, ctx)
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
		function_parse(struct, method, parser, ctx)
	# write __index function
	w(e, f"int {export_index_name(struct)}(lua_State *L) {{")
	if is_struct:
		w(e, f"\tswitch(fnv1a32(luaL_checkstring(L, 2))) {{")
		for field in struct.findall('field'):
			if field.get('array') or field.get('private'): continue
			field_name = field.get('name')
			w(e, f"\tcase {hash(field_name)}: // {field_name}")
			w(e, f"\t\t{export_push_var(field, f'luaX_check{name}(L, 1)->{field_name}', registries)};")
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
			if field_type in ctx.enums: access = ""
			if field_type in ctx.resources: access = ""
			if field_type in ctx.components: access = ""
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

	if is_struct:
		w(e, f"\tlua_newtable(L);")
		w(e, f"\tlua_pushcfunction(L, f_{name}___call);")
		w(e, f"\tlua_setfield(L, -2, \"__call\");")
		w(e, f"\tlua_setmetatable(L, -2);\n")

	w(e, f"\treturn 1;\n}}")

def register(parsers):
	parsers['struct'] = parse
	parsers['interface'] = parse
	parsers['function'] = function_parse
