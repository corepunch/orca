import io
import xml.etree.ElementTree as ET
from codegen_c import (
    w, hash, lpc, lp, lpobject_t, Axis,
    header_get_arg_type, header_write_method
)
from doc_gen import doc
from plugins.struct_plugin import write_struct_def

def property_name(name):
	def conv_name(name):
		for pat, repl in Axis:
			s2, n = pat.subn(repl, name, count=1)
			if n: return s2
		return name
	name = conv_name(name)
	return "".join([s[0].upper() + s[1:] for s in name.split('.')])

def struct_property(root, cmp, struct, parser, prefix, indent, ctx):
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
				n = n + component_property(root, cmp, prefix+name, field, parser, indent+'\t', ctx)
		else:
			n = n + component_property(root, cmp, prefix+field.get('name'), field, parser, indent+'\t', ctx)
	return n

def component_property(root, cmp, path, property, parser, indent, ctx):
	sname = property_name(path)
	ptype, cname = property.get('type'), cmp.get('name')
	struct = ctx.structs.get(ptype)
	enum = None
	decor = 'var'
	n = 0
	if not property.get('exclude-self'):
		typedata = f"kDataType{ptype[:1].upper()+ptype[1:]}"
		if struct:
			export_as = struct.get('export')
			nfields = struct_property(root, cmp, struct, None, str(), indent, ctx)
			typedata = f"kDataType{export_as}" if export_as else f"kDataTypeGroup, .TypeString=\"{ptype}\", .NumComponents={nfields}"
			decor = 'struct'
		elif ptype in ctx.enums or sname in ctx.enums:
			enum = ctx.enums.get(ptype) or ctx.enums.get(sname)
			values = [e.get('name') for e in enum.findall('enum')]
			ptype = enum.get('name')
			typedata = f"kDataTypeEnum, .TypeString=\"{','.join(values)}\""
			decor = 'enum'
		elif ptype in ctx.components:
			typedata = f"kDataTypeObject, .TypeString=\"{ptype}\""
			decor = 'object'
		if parser:
			w(parser.export, f"{indent}/* {cname}.{sname} */ DECL({hash(sname)}, {hash(cname+'.'+sname)},\n{indent}{cname}, \"{sname}\", {path}, {typedata}),")
			w(parser.props, f"\tk{cname}{sname},")
			ctx.attribs.append(sname)
			if ptype in ctx.enums:
				ctx.dtd.write(f"\t{sname} (%{ptype};) #IMPLIED\n")
			elif ptype == 'bool':
				ctx.dtd.write(f"\t{sname} (true|false) #IMPLIED\n")
			else:
				ctx.dtd.write(f"\t{sname} CDATA #IMPLIED\n")

			topic = ET.SubElement(ctx.article, 'div', {'class':'topic'})
			topic.tail = "\n"
			args = path.split('.')
			args.reverse()
			args.pop(0)

			registries = {'structs': ctx.structs, 'enums': ctx.enums, 'components': ctx.components, 'resources': ctx.resources}
			doc.add_property(topic, decor, sname, ptype, registries)
			doc.add_small_abstract(ctx.article, property.text and property.text.format(*args))

		n = 1
	return (struct_property(root, cmp, struct, parser, path+'.', indent, ctx) + n) if struct else n

def component_get_parents(component):
	parents = []
	if component.get('parent'):
		parents.append(component.get('parent'))
	if component.get('concept'):
		parents.append(component.get('concept'))
	return parents

def parse(root, component, parser, ctx):
	e, h, s = parser.export, parser.header, parser.props
	cname = component.get('name')

	ctx.dtd.write(f"<!ENTITY % {component.get('name')}Attribs \"\n")
	ctx.attribs.clear()

	if component.get('parent'):
		ctx.dtd.write(f"\t%{component.get('parent')}Attribs;\n");
		if component.get('concept'):
			ctx.dtd.write(f"\t%{component.get('concept')}Attribs;\n");
	else:
		ctx.dtd.write(f"\tName CDATA #REQUIRED\n");
		ctx.dtd.write(f"\tStyle CDATA #IMPLIED\n");
		ctx.dtd.write(f"\tIsDisabled (true|false) #IMPLIED\n");
		ctx.dtd.write(f"\tStateManager CDATA #IMPLIED\n");

	w(h, f"typedef struct {cname} {cname}, *{cname}Ptr;")
	w(h, f"typedef struct {cname} const *{cname}CPtr;")
	write_struct_def(root, component, parser.header, ctx)
	for method in component.findall('method'):
		header_write_method(component, method, parser.header, ctx)
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
				component_property(root, component, '%s[%d]'%(pname,n), property, parser, '\t', ctx)
		else:
			component_property(root, component, pname, property, parser, '\t', ctx)

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
			if ptype in ctx.enums and not pdefault.lstrip('-').isdigit():
				w(e, f"\t.{pname} = k{ptype}{pdefault},")
			elif ptype in ctx.structs:
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

	ctx.dtd.write("\">\n")

	def is_(comp, _2d):
		if comp.get('name')=="Viewport3D":
			return False
		if comp and _2d in comp.get('name'):
			return True
		if comp.get('parent') is None:
			return False
		return is_(ctx.components.get(comp.get('parent')), _2d)

	for att in ctx.attribs:
		ctx.dtd.write(f"<!ELEMENT {cname}.{att} (#PCDATA | Binding)*>\n")
		ctx.dtd.write(f"<!ATTLIST {cname}.{att} Mode (expression|oneway) #IMPLIED Attribute CDATA #IMPLIED Enabled (true|false) #IMPLIED>\n")

	attribs = [f"{cname}.{att}" for att in ctx.attribs]

	if component.get("parent"):
		attribs.append(f"%{component.get('parent')}Elements;")

	if component.get("concept"):
		attribs.append(f"%{component.get('concept')}Elements;")

	ctx.dtd.write(f"<!ENTITY % {cname}Elements \"{'|'.join(attribs)}\">\n")

	ctx.dtd.write(f"<!ELEMENT {cname} ")
	elm = f"%{cname}Elements;"
	if is_(component, '2D'):
		arr = [key for key, value in ctx.components.items() if is_(value, "2D")]+["Viewport3D","Resource","StyleSheet","LayerPrefabPlaceholder",elm]
		ctx.dtd.write(f"({'|'.join(arr)})*>\n")
	elif is_(component, '3D'):
		arr = [key for key, value in ctx.components.items() if is_(value, "3D")]+["Resource","StyleSheet","ObjectPrefabPlaceholder",elm]
		ctx.dtd.write(f"({'|'.join(arr)})*>\n")
	elif component.get('name')=="Viewport3D":
		arr = [key for key, value in ctx.components.items()]+["Resource","StyleSheet","ObjectPrefabPlaceholder",elm]
		ctx.dtd.write(f"({'|'.join(arr)})*>\n")
	else:
		ctx.dtd.write("EMPTY>\n")
	ctx.dtd.write(f"<!ATTLIST {cname} %{cname}Attribs;>\n")

def register(parsers):
	parsers['component'] = parse
