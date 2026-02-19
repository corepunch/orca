import xml.etree.ElementTree as ET
from codegen_c import w, _e
from doc_gen import doc

def parse(_, enums, parser, ctx):
	e, h = parser.export, parser.header
	ename = enums.get('name')

	article = doc.add_article(ctx.content, ename, 'None', 'None')
	doc.add_header(article, "Enum", ename, enums.findtext('summary'))
	doc.add_overview(article, enums.find('details'))

	link = ET.SubElement(ctx.sidebar, 'a', {'class':'leaf-link', 'href':f'#{ename}'})
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

	w(h, f"typedef enum {enums.get('name')} {{")
	for enum in enums.findall('enum'):
		enum_name = f"k{enums.get('name')+enum.get('name')}"
		if enum.text:
			w(h, f"\t{enum_name}, /// {enum.text}")
		else:
			w(h, f"\t{enum_name},")

		topic = ET.SubElement(article, 'div', {'class':'topic'})
		topic.tail = "\n"
		registries = {'structs': ctx.structs, 'enums': ctx.enums, 'components': ctx.components, 'resources': ctx.resources}
		doc.add_property(topic, 'case', enum.get('name'), '', registries)
		doc.add_small_abstract(topic, enum.text)

	w(h, f"}} {_e(enums.get('name'))};\n")

	ctx.dtd.write(f"<!ENTITY % {enums.get('name')} \"{'|'.join([e.get('name') for e in enums.findall('enum')])}\">\n")

def register(parsers):
	parsers['enums'] = parse
