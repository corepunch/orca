"""
HTML Plugin - Generates HTML documentation
"""

import xml.etree.ElementTree as ET
from . import BasePlugin

def tokenize(code):
	"""Tokenize code for syntax highlighting"""
	import re
	token_re = re.compile(
		r'\s*"[^"\n]*"'   # double-quoted strings (no newlines inside)
		r'|--[^\n]*'      # single-line comments
		r"|\n"            # newline
		r"|\t"            # tab
		r"|\w+"           # identifiers/keywords
		r"|\d+"           # numbers
		r"|[^\w\d\n\t]"   # everything else (punctuation, operators)
	)
	for match in token_re.finditer(code):
		yield match.group(0)

rules = {
	"lua": "require|and|break|do|else|elseif|end|false|for|function|goto|if|in|local|nil|not|or|repeat|return|then|true|until|while"
}

def wrap_backticks(elem):
	"""Convert backtick syntax to HTML code elements"""
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

class DocumentationGenerator:
	"""HTML documentation generator"""
	def __init__(self):
		self.g_structs = {}
		self.g_enums = {}
		self.g_components = {}
	
	def set_globals(self, structs, enums, components):
		"""Set global state for documentation generation"""
		self.g_structs = structs
		self.g_enums = enums
		self.g_components = components
	
	def add_header(self, div, type, name, brief):
		ET.SubElement(div, 'div', {'class':'eyebrow'}).text = type+"\n"
		ET.SubElement(div, 'h1', {'class':'header'}).text = name+"\n"
		if brief:
			ET.SubElement(div, 'div', {'class':'abstract'}).text = brief+"\n"

	def add_code(self, div, headers, type, name, parents):
		code = ET.SubElement(div, 'code', {'class':'source'})

		for key, value in headers:
			top = ET.SubElement(code, "div", {'class':'token-keyword'})
			ET.SubElement(top, 'span', {'class':'token-keyword'}).text = key
			ET.SubElement(top, 'span', {'class':'token-string'}).text = value

		ET.SubElement(code, "span", {'class':'token-keyword'}).text = type
		ET.SubElement(code, "span", {'class':'token-identifier'}).text = name

		if not parents:
			return

		ET.SubElement(code, 'span', {'class':'token-keyword'}).text = " inherits "
		cont = ET.SubElement(code, 'ui', {'class':'comma-list'})
		for n in parents.split(","):
			li = ET.SubElement(cont, 'li')
			ET.SubElement(li, 'a', {'href':'#'+n.strip(), 'class':'name base'}).text = n

	def add_overview(self, div, overview):
		if overview is None:
			return
		overview = wrap_backticks(overview)
		ET.SubElement(div, 'h2', {'class':'title'}).text = 'Overview'
		
		overview.tag = "div"
		overview.attrib['class'] = 'abstract'
		div.append(overview)

		for node in overview.findall('.//snippet[@src]'):
			with open(node.get('src')) as f:
				node.append(self.highlight(f.read(), "lua"))
			node.tag = 'div'

	def print_abstract(self, div, doc):
		for i, n in enumerate(doc.split('`')):
			if i % 2:
				ET.SubElement(div, 'code', {'class':'token-keyword'}).text = n
			else:
				ET.SubElement(div, 'span').text = n

	def highlight(self, code, lang="lua"):
		root = ET.Element("code", {"class": "source code-listing"})
		kws = rules.get(lang, set()).split('|')
		for tok in tokenize(code):
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
				if tok[0].isupper():  # PascalCase
					ET.SubElement(root, "span", {"class": "syntax-type"}).text = tok
				else:
					ET.SubElement(root, "span", {"class": "syntax-variable"}).text = tok
			elif tok == '\n':
				ET.SubElement(root, "br")
			elif tok == '\t':
				ET.SubElement(root, "span").text = "\u00A0"*4
			elif root:
				if ('{' in tok or '(' in tok) and root[-1].attrib['class'] in ["syntax-variable", "syntax-identifier"]:
					root[-1].attrib['class'] = "syntax-function"
				ET.SubElement(root, "span", {"class": "syntax-identifier"}).text = tok
		return root
	
	def add_small_abstract(self, topic, abstract):
		if not abstract:
			return
		self.print_abstract(ET.SubElement(topic, 'div', {'class':'small-abstract'}), abstract+'\n')

	def add_property(self, topic, decor, name, type):
		code = ET.SubElement(topic, 'code', {'class':'decorator'})
		ET.SubElement(code, 'span').text = decor + " "
		nm = ET.SubElement(code, 'span', {'class':'identifier'})
		nm.text = name
		if type:
			nm.tail = ': '
			if type in self.g_enums or type in self.g_structs or type in self.g_components:
				ET.SubElement(code, 'a', {'class': 'type-identifier-link', 'href': '#'+type}).text = type
			else:
				ET.SubElement(code, 'span').text = type

	def add_article(self, body, name, parent, namespace):
		return ET.SubElement(body, 'div', {
			"id":name,
			'class':'class', 
			'data-name':name, 
			'data-inherits':parent, 
			'data-xmlns':namespace
		})

	def add_function(self, topic, function, returns):
		from .common import camel_case
		name = camel_case(function.get('name'))
		code = ET.SubElement(topic, 'code', {'class':'decorator'})
		ET.SubElement(code, 'span').text = 'func '
		nm = ET.SubElement(code, 'span', {'class':'identifier'})
		nm.text = name
		params = function.findall('arg')
		if params:
			nm.tail = '('
			while params:
				arg = params.pop(0)
				aname = ET.SubElement(code, 'span', {'class':'identifier'})
				aname.text = arg.get('name')
				aname.tail = ':'
				atype = ET.SubElement(code, 'span', {'class':'decorator'})
				atype.text = arg.get('type')
				atype.tail = params and ', ' or ')'
		else:
			nm.tail = '()'
		if returns is not None:
			ET.SubElement(code, 'span', {'class':'decorator'}).text = ' ->'
			ET.SubElement(code, 'span').text = " " + returns.get('type')

class HtmlPlugin(BasePlugin):
	"""Plugin for generating HTML documentation"""
	def __init__(self):
		# HTML generation uses global state (g_html, g_sidebar, g_content)
		# This plugin doesn't need a file handle as it's managed separately
		self.doc = DocumentationGenerator()
	
	def set_globals(self, structs, enums, components):
		"""Set global state on documentation generator"""
		self.doc.set_globals(structs, enums, components)
	
	def write_to_global(self, element_type, *args, **kwargs):
		"""Write to the global HTML structure"""
		# This would be used for adding elements to g_sidebar, g_content, etc.
		pass
