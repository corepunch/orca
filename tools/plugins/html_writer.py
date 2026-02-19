"""HTML documentation writer plugin."""

import xml.etree.ElementTree as ET

from . import Plugin, utils, Workspace


class doc:
	"""Static helper methods for building HTML documentation elements."""

	def add_header(div, type, name, brief):
		ET.SubElement(div, 'div', {'class': 'eyebrow'}).text = type + "\n"
		ET.SubElement(div, 'h1', {'class': 'header'}).text = name + "\n"
		if brief:
			ET.SubElement(div, 'div', {'class': 'abstract'}).text = brief + "\n"

	def add_code(div, headers, type, name, parents):
		code = ET.SubElement(div, 'code', {'class': 'source'})
		for key, value in headers:
			top = ET.SubElement(code, "div", {'class': 'token-keyword'})
			ET.SubElement(top, 'span', {'class': 'token-keyword'}).text = key
			ET.SubElement(top, 'span', {'class': 'token-string'}).text = value
		ET.SubElement(code, "span", {'class': 'token-keyword'}).text = type
		ET.SubElement(code, "span", {'class': 'token-identifier'}).text = name
		if not parents:
			return
		ET.SubElement(code, 'span', {'class': 'token-keyword'}).text = " inherits "
		cont = ET.SubElement(code, 'ui', {'class': 'comma-list'})
		for n in parents.split(","):
			li = ET.SubElement(cont, 'li')
			ET.SubElement(li, 'a', {'href': '#' + n.strip(), 'class': 'name base'}).text = n

	def add_overview(div, overview):
		if overview is None:
			return
		overview = utils.wrap_backticks(overview)
		ET.SubElement(div, 'h2', {'class': 'title'}).text = 'Overview'
		overview.tag = "div"
		overview.attrib['class'] = 'abstract'
		div.append(overview)
		for node in overview.findall('.//snippet[@src]'):
			with open(node.get('src')) as f:
				node.append(doc.highlight(f.read(), "lua"))
			node.tag = 'div'

	def print_abstract(div, text):
		for i, n in enumerate(text.split('`')):
			if i % 2:
				ET.SubElement(div, 'code', {'class': 'token-keyword'}).text = n
			else:
				ET.SubElement(div, 'span').text = n

	def highlight(code, lang="lua"):
		root = ET.Element("code", {"class": "source code-listing"})
		kws = utils._token_rules.get(lang, set()).split('|')
		for tok in utils.tokenize(code):
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
				if tok[0].isupper():
					ET.SubElement(root, "span", {"class": "syntax-type"}).text = tok
				else:
					ET.SubElement(root, "span", {"class": "syntax-variable"}).text = tok
			elif tok == '\n':
				ET.SubElement(root, "br")
			elif tok == '\t':
				ET.SubElement(root, "span").text = "\u00A0" * 4
			elif root is not None:
				if ('{' in tok or '(' in tok) and root[-1].attrib['class'] in [
					"syntax-variable", "syntax-identifier"
				]:
					root[-1].attrib['class'] = "syntax-function"
				ET.SubElement(root, "span", {"class": "syntax-identifier"}).text = tok
		return root

	def add_small_abstract(topic, abstract):
		if not abstract:
			return
		doc.print_abstract(
			ET.SubElement(topic, 'div', {'class': 'small-abstract'}), abstract + '\n'
		)

	def add_property(topic, decor, name, type):
		code = ET.SubElement(topic, 'code', {'class': 'decorator'})
		ET.SubElement(code, 'span').text = decor + " "
		nm = ET.SubElement(code, 'span', {'class': 'identifier'})
		nm.text = name
		if type:
			nm.tail = ': '
			if type in Workspace.enums or type in Workspace.structs or type in Workspace.components:
				ET.SubElement(code, 'a', {'class': 'type-identifier-link', 'href': '#' + type}).text = type
			else:
				ET.SubElement(code, 'span').text = type

	def add_article(body, name, parent, namespace):
		return ET.SubElement(body, 'div', {
			"id": name,
			'class': 'class',
			'data-name': name,
			'data-inherits': parent,
			'data-xmlns': namespace,
		})

	def add_function(topic, function, returns):
		name = utils.camel_case(function.get('name'))
		code = ET.SubElement(topic, 'code', {'class': 'decorator'})
		ET.SubElement(code, 'span').text = 'func '
		nm = ET.SubElement(code, 'span', {'class': 'identifier'})
		nm.text = name
		params = function.findall('arg')
		if params:
			nm.tail = '('
			while params:
				arg = params.pop(0)
				aname = ET.SubElement(code, 'span', {'class': 'identifier'})
				aname.text = arg.get('name')
				aname.tail = ':'
				atype = ET.SubElement(code, 'span', {'class': 'decorator'})
				atype.text = arg.get('type')
				atype.tail = params and ', ' or ')'
		else:
			nm.tail = '()'
		if returns is not None:
			ET.SubElement(code, 'span', {'class': 'decorator'}).text = ' ->'
			ET.SubElement(code, 'span').text = " " + returns.get('type')


class HtmlWriter(Plugin):
	"""Generates an HTML documentation page from module XML."""

	def __init__(self):
		self.article = None
		self.html = ET.Element('html', attrib={'xmlns': 'http://www.w3.org/2001/XMLSchema'})
		self.head = ET.SubElement(self.html, 'head')
		ET.SubElement(self.head, 'link', {"rel": "stylesheet", "href": "doc.css"})
		ET.SubElement(self.head, 'link', {
			"rel": "stylesheet",
			"href": "https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css",
		})
		ET.SubElement(self.head, 'meta', {'charset': "UTF-8"})
		self.body = ET.SubElement(self.html, 'body')
		self.wrapper = ET.SubElement(
			self.body, 'div',
			{'class': 'adjustable-sidebar-width full-width-container topic-wrapper'},
		)
		self.sidebar = ET.SubElement(self.wrapper, 'div', {'class': 'sidebar'})
		self.content = ET.SubElement(self.wrapper, 'main', {'class': 'content'})
		self.output = "../docs/index.html"

	def open(self, _, root):
		tech = ET.SubElement(self.sidebar, 'div', {'class': 'technology-title'})
		i = ET.SubElement(tech, 'span', {'class': 'fa-solid fa-folder-open icon'})
		i.tail = f"{root.get('namespace')}.{root.get('name')}\n"

	def finalize(self):
		tree = ET.ElementTree(self.html)
		tree.write(self.output, encoding="utf-8", method="html")

	def on_struct(self, root, struct):
		sname, sbrief = struct.get('name'), struct.findtext('summary')
		self.article = doc.add_article(
			self.content, sname,
			struct.get('parent') or 'None',
			struct.findtext('xmlns') or 'None',
		)
		atts = []
		doc.add_header(self.article, struct.tag.capitalize(), sname, sbrief)
		doc.add_code(
			self.article, atts, "struct ",
			"%s.%s" % (root.get('name'), sname),
			struct.get('parent') or (struct.tag == 'component' and 'object'),
		)
		doc.add_overview(self.article, struct.find('details'))

		icons = {
			'struct':    'gear',
			'component': 'layer-group',
			'interface': 'atom',
		}
		link = ET.SubElement(self.sidebar, 'a', {'class': 'leaf-link', 'href': f'#{sname}'})
		ET.SubElement(link, 'i', {'class': f'fa-solid fa-{icons.get(struct.tag)} icon'})
		ET.SubElement(link, 'span').text = sname + '\n'

		if struct.findall('property') or struct.findall('methods'):
			ET.SubElement(self.article, 'h2', {'class': 'title'}).text = 'Properties'

		for field in struct.findall('property') + struct.findall('field'):
			fname, ftype = field.get('name'), utils.header_get_arg_type(field)
			decor = 'var'
			ftype = field.get('type')
			if ftype in Workspace.enums:      decor = 'enum'
			if ftype in Workspace.components: decor = 'object'
			if ftype in Workspace.resources:  decor = 'resource'
			if field.tag == 'field':
				topic = ET.SubElement(self.article, 'div', {'class': 'topic'})
				topic.tail = "\n"
				doc.add_property(topic, decor, fname, ftype)
				doc.add_small_abstract(self.article, field.text)

		for method in struct.findall('method'):
			self.on_function(struct, method)

	def on_function(self, _, method):
		returns = method.find('returns')
		topic = ET.SubElement(self.article, 'div', {'class': 'topic'})
		doc.add_function(topic, method, returns and returns.get('type'))
		doc.add_small_abstract(topic, method.findtext('summary'))

	def on_property(self, property, _, path):
		property_type = property.get('type')
		readable_name = (
			property.get('name') if property.tag == 'shorthand' else utils.property_name(path)
		)
		struct = Workspace.structs.get(property_type)
		decor = 'var'
		if property_type == 'Edges':
			return
		if struct is not None:
			decor = 'struct'
		elif property_type in Workspace.enums or readable_name in Workspace.enums:
			enum = Workspace.enums.get(property_type) or Workspace.enums.get(readable_name)
			property_type = enum.get('name')
			decor = 'enum'
		elif property_type in Workspace.components:
			decor = 'object'

		topic = ET.SubElement(self.article, 'div', {'class': 'topic'})
		topic.tail = "\n"
		args = path.split('.')
		args.reverse()
		args.pop(0)
		doc.add_property(topic, decor, readable_name, property_type)
		doc.add_small_abstract(self.article, property.text and property.text.format(*args))

	def on_component(self, root, component):
		self.on_struct(root, component)
		utils.enum_component_properties(component, self.on_property)

	def on_enums(self, root, enums):
		ename = enums.get('name')
		article = doc.add_article(self.content, ename, 'None', 'None')
		doc.add_header(article, "Enum", ename, enums.findtext('summary'))
		doc.add_overview(article, enums.find('details'))

		link = ET.SubElement(self.sidebar, 'a', {'class': 'leaf-link', 'href': f'#{ename}'})
		ET.SubElement(article, 'h2', {'class': 'title'}).text = 'Constants'
		ET.SubElement(link, 'i', {'class': f'fa-solid fa-bars icon'})
		ET.SubElement(link, 'span').text = ename + '\n'

		for enum in enums.findall('enum'):
			topic = ET.SubElement(article, 'div', {'class': 'topic'})
			topic.tail = "\n"
			doc.add_property(topic, 'case', enum.get('name'), '')
			doc.add_small_abstract(topic, enum.text)

	def on_interface(self, *args):
		return self.on_struct(*args)
