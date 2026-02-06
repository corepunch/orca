import xml.etree.ElementTree as ET
import sys, re, os, io
import codegen.struct
import codegen.common
import codegen.doc
import codegen.xsd
import codegen.utils
import codegen.module


includes = """#include <include/orca.h>
#include <include/api.h>
#include <%s>\n
// clang-format off\n
#define DECL(ID, CLASS, NAME, FIELD, TYPE,...) { \\
\tID, NAME, offsetof(struct CLASS, FIELD), \\
\tsizeof(((struct CLASS *)NULL)->FIELD), TYPE, ##__VA_ARGS__ }\n\n"""

def write_class_enums(f, classname, vars):
	f.write("enum %sProperties {\n" % classname)
	for _, name, _, _ in vars:
		f.write("\t%s,\n" % codegen.utils.enum_name(classname, name))
	f.write("\tk%sNumProperties,\n" % classname)
	f.write("};\n\n")

# Function to process the header file
def process_header_file(input_file, output_file):
	with open(input_file, 'r') as file:
		content = file.read()

	classes2 = list(re.finditer(codegen.common.CLASS_PATTERN, content, re.VERBOSE))
	header = open(input_file.replace(".h", "_properties.h"), 'w')
	classprops = {}

	for name, body in [cl.group('name', 'body') for cl in classes2]:
		classprops[name] = body

	# Simplified regex pattern using \w+ to match alphanumeric and underscores
	pattern = r'(?:CLASS_TYPE\(\s*(\w+)\s*,\s*(\w+)\s*,\s*([^)]+)+\))'

	# Callback function for re.sub to replace the third entry with the hash
	def replace_with_hash(match):
		class_name = match.group(1)	# The first entry (e.g., UIViewController)
		hash_value = codegen.utils.fnv1a32(class_name)
		hash_hex = f'{hash_value:08x}'	# Convert to hex, zero-padded to 8 digits
		buf = io.StringIO()
		write_class_enums(buf, match.group(1), codegen.common.parse_fields(classprops.get(match.group(1), "")))
		header.write(buf.getvalue())
		return f'CLASS_TYPE({match.group(1)}, 0x{hash_hex}, {match.group(3)})'

	# Replace using the pattern and callback function
	new_content = re.sub(pattern, replace_with_hash, content)

	# Write the modified content to a new file
	with open(output_file, 'w') as file:
		file.write(new_content)

	header.close()

with open(sys.argv[1] + ".pkg") as f:
	content = f.read()
	codegen.struct.parse_all_structs(content)
	schema = ET.Element('xs:schema', attrib={
		'xmlns:xs': 'http://www.w3.org/2001/XMLSchema', 
		'targetNamespace': 'http://schemas.corepunch.com/orca/2006/xml/'+sys.argv[1],
		'xmlns': 'http://schemas.corepunch.com/orca/2006/xml/'+sys.argv[1],
		"elementFormDefault": "qualified",
	})

	complexType = ET.SubElement(schema, 'xs:complexType', attrib={'name': "Object"})
	ET.SubElement(complexType, 'xs:attribute', attrib={'name': "name", "type": "xs:string"})#, "use": "required"})
	ET.SubElement(complexType, 'xs:attribute', attrib={'name': "stateManager", "type": "xs:string"})

	html = ET.Element('html', attrib={'xmlns':'http://www.w3.org/2001/XMLSchema' })
	head = ET.SubElement(html, 'head')
	ET.SubElement(head, 'link', { "rel":"stylesheet", "href":"doc.css" })
	ET.SubElement(head, 'meta', {'charset':"UTF-8"})
	body = ET.SubElement(html, 'body')
	wrapper = ET.SubElement(body, 'div', {'class': 'adjustable-sidebar-width full-width-container topic-wrapper'})
	sidebar = ET.SubElement(wrapper, 'div', {'class': 'sidebar'})
	hero = ET.SubElement(wrapper, 'main', {'class': 'content'})

	for module in re.findall("module\s+([a-zA-Z0-9]+)\s*=\s*\"([^\"]+)\"", content, re.MULTILINE):
		with open(module[1], "r") as modf:
			dir = os.path.dirname(module[1])
			raw = modf.read()
			# Regular expression to extract the URL after 'xmlns="...'
			matches = re.findall(r'//\s+@xmlns\s(http[s]?://[^\s"]+)', raw, re.VERBOSE)
			content = raw
			with open(os.path.join(dir, "module_%s.c" % module[0]), "w") as out:
				print(os.path.join(dir, "module_%s.c" % module[0]))
				name = os.path.basename(module[0])
				name = os.path.splitext(name)[0]
				out.write(includes % re.search("(source/.+)", module[1])[1])
				codegen.module.read_classes(out, content, module[0], matches and matches[0] or "")
			codegen.xsd.write_xsd(schema, content)
			codegen.doc.write_doc(hero, sidebar, content, module[1], matches and matches[0] or "")
		process_header_file(module[1], module[1])

	tree = ET.ElementTree(html)
	# ET.indent(tree, space="\t", level=0)
	tree.write('doc.html', encoding="utf-8", method="html")

	tree = ET.ElementTree(schema)
	ET.indent(tree, space="\t", level=0)
	if len(sys.argv) > 2:
		tree.write(sys.argv.get(2), encoding="utf-8")
	else:
		tree.write(sys.argv[1] + ".xsd", encoding="utf-8")
