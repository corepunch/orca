#include <source/filesystem/fs_local.h>
#include <libxml/tree.h>

// Both exporters share reflection and prefab handling; only the textual
// representation differs. No XML text is parsed when cloning a scene.
static void
write_string(xmlBufferPtr out, const xmlChar *text)
{
  xmlBufferAdd(out, XMLSTR("\""), 1);
  for (const xmlChar *p = text; p && *p; p++) {
    switch (*p) {
      case '"': xmlBufferCat(out, XMLSTR("\\\"")); break;
      case '\\': xmlBufferCat(out, XMLSTR("\\\\")); break;
      case '\n': xmlBufferCat(out, XMLSTR("\\n")); break;
      case '\r': xmlBufferCat(out, XMLSTR("\\r")); break;
      case '\t': xmlBufferCat(out, XMLSTR("\\t")); break;
      default: xmlBufferAdd(out, p, 1); break;
    }
  }
  xmlBufferAdd(out, XMLSTR("\""), 1);
}

static void
write_node(xmlBufferPtr out, xmlNodePtr node)
{
  xmlBufferCat(out, XMLSTR("("));
  xmlBufferCat(out, node->name);
  for (xmlAttrPtr attr = node->properties; attr; attr = attr->next) {
    xmlChar *value = xmlNodeListGetString(node->doc, attr->children, 1);
    xmlBufferCat(out, XMLSTR(" "));
    xmlBufferCat(out, attr->name);
    xmlBufferCat(out, XMLSTR("="));
    write_string(out, value);
    xmlFree(value);
  }
  for (xmlNodePtr child = node->children; child; child = child->next) {
    if (child->type == XML_ELEMENT_NODE) {
      xmlBufferCat(out, XMLSTR("\n"));
      write_node(out, child);
    } else if (child->type == XML_TEXT_NODE || child->type == XML_CDATA_SECTION_NODE) {
      xmlBufferCat(out, XMLSTR(" "));
      write_string(out, child->content);
    }
  }
  xmlBufferCat(out, XMLSTR(")"));
}

const char *
FS_SerializeObjectToLispString(struct Object const *object)
{
  xmlDocPtr tree = FS_SerializeObjectTree(object);
  if (!tree) return NULL;
  xmlBufferPtr out = xmlBufferCreate();
  if (!out) { xmlFreeDoc(tree); return NULL; }
  write_node(out, xmlDocGetRootElement(tree));
  const char *result = strdup((const char *)xmlBufferContent(out));
  xmlBufferFree(out);
  xmlFreeDoc(tree);
  return result;
}
