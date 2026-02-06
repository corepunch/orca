#include <ctype.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <include/api.h>

#include "xml_local.h"

#define API_TYPE_XML_DOC "XmlDoc"
#define API_TYPE_XML_NODE "XmlNode"

extern bool_t FS_FileExists(lpcString_t path);
extern const char* FS_PathFromModule(const char* module);

// Iterator function for child nodes
static int lua_xml_child_iterator(lua_State* L)
{

  xmlNodePtr child = (xmlNodePtr)lua_touserdata(L, lua_upvalueindex(1));
  xmlChar* filter = (xmlChar*)luaL_optstring(L, lua_upvalueindex(2), NULL);
  long type = luaL_optinteger(L, lua_upvalueindex(3), XML_ELEMENT_NODE);

  while (child && ((type && child->type != type) ||
                   (filter && (xmlStrcmp(filter, child->name) != 0)))) {
    child = child->next;
  }

  if (child == NULL) {
    return 0; // End of iteration
  }

  // Push the child node as lightuserdata
  xmlNodePtr* child_node = lua_newuserdata(L, sizeof(xmlNodePtr));
  *child_node = child;
  luaL_setmetatable(L, API_TYPE_XML_NODE);
  xmlChar* name;
  if (type == XML_PI_NODE) {
    lua_pushstring(L, (lpcString_t)child->content);
  } else if ((name = xmlGetProp(child, XMLSTR("id")))) {
    lua_pushstring(L, (lpcString_t)name);
    if (type == XML_PI_NODE) {
      xmlFree(name);
    }
  } else {
    lua_pushnil(L);
  }
  if (child->ns) {
    lua_pushstring(L, (lpcString_t)child->ns->href);
  } else {
    lua_pushnil(L);
  }

  // Move to the next sibling
  child = child->next;
  lua_pushlightuserdata(L, child);
  lua_replace(L, lua_upvalueindex(1));

  return 3; // Return the child node
}

// Function to read XML document
static int lua_xml_read_document(lua_State* L)
{
  lpcString_t filename = luaL_checkstring(L, 1);
  xmlDocPtr* doc = lua_newuserdata(L, sizeof(*doc));
  luaL_setmetatable(L, API_TYPE_XML_DOC);
  *doc = xmlReadFile(filename, NULL, 0);
  if (!*doc) {
    return luaL_error(L, "Failed to parse XML document: %s", filename);
  }
  return 1;
}

#include <libxml/HTMLparser.h>

// void printNodeStructure(xmlNode *node, int depth) {
//	while (node) {
//		// Print indentation for the current depth
//		for (int i = 0; i < depth; i++) {
//			Con_Error("  ");
//		}
//
//		// Print the node type
//		Con_Error("Node type: ");
//		switch (node->type) {
//		case XML_ELEMENT_NODE:
//			Con_Error("Element");
//			break;
//		case XML_TEXT_NODE:
//			Con_Error("Text");
//			break;
//		case XML_COMMENT_NODE:
//			Con_Error("Comment");
//			break;
//		case XML_CDATA_SECTION_NODE:
//			Con_Error("CDATA");
//			break;
//		case XML_PI_NODE:
//			Con_Error("Processing Instruction");
//			break;
//		default:
//			Con_Error("Other (%d)", node->type);
//		}
//
//		// Print the node's name if it's an element
//		if (node->type == XML_ELEMENT_NODE && node->name) {
//			Con_Error(", Tag: <%s>", node->name);
//		}
//
//		Con_Error("\n");
//
//		if (node->type == XML_ELEMENT_NODE && node->properties) {
//			xmlAttr *attr = node->properties;
//			while (attr) {
//				// Print indentation for attributes
//				for (int i = 0; i < depth + 1; i++) {
//					Con_Error("  ");
//				}
//
//				// Print attribute name and value
//				Con_Error("Attribute: %s, Value: ", attr->name);
//				if (attr->children && attr->children->content) {
//					Con_Error("\"%s\"",
// attr->children->content); 				} else {
// Con_Error("(null)");
//				}
//				Con_Error("\n");
//
//				// Move to the next attribute
//				attr = attr->next;
//			}
//		}
//
//		// Recursively print children
//		if (node->children) {
//			printNodeStructure(node->children, depth + 1);
//		}
//
//		// Move to the next sibling
//		node = node->next;
//	}
// }

static int lua_xml_parse_document(lua_State* L)
{
  lpcString_t text = luaL_checkstring(L, 1);
  lpcString_t filename = luaL_optstring(L, 2, "[Buffer]");
  htmlDocPtr* doc = lua_newuserdata(L, sizeof(*doc));
  luaL_setmetatable(L, API_TYPE_XML_DOC);
  *doc = xmlReadMemory(text, (int)strlen(text), filename, NULL, XML_FLAGS);
  // HTML_PARSE_NOIMPLIED|HTML_PARSE_PEDANTIC|HTML_PARSE_NOBLANKS);
  //	if (*doc) {
  //		printNodeStructure(xmlDocGetRootElement(*doc), 0);
  //	}
  if (!*doc) {
    return luaL_error(L, "Failed to parse XML document: %s", filename);
  }
  return 1;
}

static int lua_xml_new_document(lua_State* L)
{
  lpcString_t version = luaL_checkstring(L, 1);
  xmlDocPtr* doc = lua_newuserdata(L, sizeof(*doc));
  luaL_setmetatable(L, API_TYPE_XML_DOC);
  *doc = xmlNewDoc((xmlChar const*)version);
  return 1;
}

// Function to free document
static int lua_xml_free_document(lua_State* L)
{
  xmlDocPtr* doc = luaL_checkudata(L, 1, API_TYPE_XML_DOC);
  if (*doc) {
    xmlFreeDoc(*doc);
  }
  return 0;
}

static int lua_xml_save(lua_State* L)
{
  xmlDocPtr* doc = luaL_checkudata(L, 1, API_TYPE_XML_DOC);
  lpcString_t filename = luaL_checkstring(L, 2);
  xmlSaveFormatFile(filename, *doc, 1);
  return 0;
}

static int lua_xml_doc_str(lua_State* L)
{
  xmlDocPtr* doc = luaL_checkudata(L, 1, API_TYPE_XML_DOC);
  xmlBufferPtr buf = xmlBufferCreate();
  if (!buf) {
    return lua_error(L);
  }
  xmlOutputBufferPtr out = xmlOutputBufferCreateBuffer(buf, NULL);
  if (!out) {
    xmlBufferFree(buf);
    return lua_error(L);
  }
  xmlSaveFormatFileTo(out, *doc, NULL, 1);
  lua_pushlstring(L, (lpcString_t)xmlBufferContent(buf), xmlBufferLength(buf));
  xmlBufferFree(buf);
  return 1;
}

static int lua_xml_doc_index(lua_State* L)
{
  xmlDocPtr* doc = luaL_checkudata(L, 1, API_TYPE_XML_DOC);
  lpcString_t name = luaL_checkstring(L, 2);
  if (!strcmp(name, "root")) {
    xmlNodePtr* root = lua_newuserdata(L, sizeof(*doc));
    *root = xmlDocGetRootElement(*doc);
    luaL_setmetatable(L, API_TYPE_XML_NODE);
    return 1;
  }
  if (!strcmp(name, "filename")) {
    lpcString_t filename = (lpcString_t)(*doc)->URL;
    if (filename) {
      lua_pushstring(L, filename);
    } else {
      lua_pushnil(L);
    }
  }
  if (!strcmp(name, "children")) {
    lua_pushlightuserdata(L, xmlDocGetRootElement(*doc));
    lua_pushcclosure(L, lua_xml_child_iterator, 1);
    return 1;
  }
  if (!strcmp(name, "save")) {
    lua_pushcfunction(L, lua_xml_save);
    return 1;
  }
  
  xmlNodePtr root = xmlDocGetRootElement(*doc);
  
  if (root && !xmlStrcmp(root->name, XMLSTR("Plugin"))) {
    path_t tmp={0};
    snprintf(tmp, sizeof(tmp), "%s.%s", (*doc)->name, name);
//    xmlWith(xmlChar, name, xmlGetProp(root, XMLSTR("name")), xmlFree) {
//      snprintf(tmp, sizeof(tmp), "%s.%s", (lpcString_t )name, name);
//    }
    lua_getglobal(L, "require");
    lua_pushstring(L, tmp);
    if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
      return 1;
    } else {
      return 1;
    }
  } else {
    return luaL_error(L, "Incorrect field name %s for %s", name, API_TYPE_XML_DOC);
  }
}

static int lua_xml_doc_newindex(lua_State* L)
{
  xmlDocPtr* doc = luaL_checkudata(L, 1, API_TYPE_XML_DOC);
  lpcString_t name = luaL_checkstring(L, 2);
  if (!strcmp(name, "root")) {
    xmlNodePtr* node = luaL_checkudata(L, 2, API_TYPE_XML_NODE);
    xmlDocSetRootElement(*doc, *node);
    return 0;
  }
  return luaL_error(
    L, "Incorrect field name %s for %s", name, API_TYPE_XML_DOC);
}

// xml node

// Iterator function for attributes
static int lua_xml_attr_iterator(lua_State* L)
{
  xmlAttr* attr = (xmlAttr*)lua_touserdata(L, lua_upvalueindex(1));
  if (attr == NULL) {
    return 0; // End of iteration
  }
  // Push the attribute name and value
  lua_pushstring(L, (lpcString_t)attr->name);

  xmlChar* value = xmlNodeListGetString(attr->parent->doc, attr->children, 1);
  lua_pushstring(L, (lpcString_t)value);
  xmlFree(value);

  if (attr->ns) {
    lua_pushstring(L, (lpcString_t)attr->ns->href);
  } else {
    lua_pushnil(L);
  }

  // Move to the next attribute
  attr = attr->next;
  lua_pushlightuserdata(L, attr);
  lua_replace(L, lua_upvalueindex(1));

  return 3; // Return the key-value pair
}

// Iterator function for attributes
static int lua_xml_nsdef_iterator(lua_State* L)
{
  xmlNs* ns = (xmlNs*)lua_touserdata(L, lua_upvalueindex(1));
  if (ns == NULL) {
    return 0; // End of iteration
  }
  // Push the attribute name and value
  lua_pushstring(L, ns->prefix ? (lpcString_t)ns->prefix : "orca");
  lua_pushstring(L, (lpcString_t)ns->href);
  //	if (ns->prefix) {
  //		lua_pushstring(L, (lpcString_t )ns->prefix);
  //	} else {
  //		lua_pushnil(L);
  //	}
  //	if (ns->href) {
  //		lua_pushstring(L, (lpcString_t )ns->href);
  //	} else {
  //		lua_pushnil(L);
  //	}

  // Move to the next attribute
  ns = ns->next;
  lua_pushlightuserdata(L, ns);
  lua_replace(L, lua_upvalueindex(1));
  return 2; // Return the key-value pair
}

// Function to set node attribute
static int lua_xml_set_node_attribute(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  lpcString_t name = luaL_checkstring(L, 2);
  lpcString_t value = luaL_optstring(L, 3, NULL);
  if (value) {
    xmlSetProp(*node, BAD_CAST name, BAD_CAST value);
  } else {
    xmlUnsetProp(*node, BAD_CAST name);
  }
  return 0;
}

// Function to get node attributesd
static int lua_xml_get_node_attribute(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  xmlChar* name = BAD_CAST luaL_checkstring(L, 2);
  xmlChar* ns = BAD_CAST luaL_optstring(L, 3, NULL);
  xmlChar* value = ns ? xmlGetNsProp(*node, name, ns) : xmlGetProp(*node, name);
  if (value) {
    lua_pushstring(L, (lpcString_t)value);
    xmlFree(value);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

// Function to get child nodes as an iterator
static int lua_xml_findall(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  lpcString_t name = luaL_checkstring(L, 2);
  lua_pushlightuserdata(L, (*node)->children);
  lua_pushstring(L, name);
  lua_pushcclosure(L, lua_xml_child_iterator, 2);
  return 1;
}

// Function to get child nodes as an iterator
static int lua_xml_findallpi(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  xmlNodePtr n = *node;
  lpcString_t name = luaL_checkstring(L, 2);
  while (n->prev)
    n = n->prev;
  lua_pushlightuserdata(L, n);
  lua_pushstring(L, name);
  lua_pushinteger(L, XML_PI_NODE);
  lua_pushcclosure(L, lua_xml_child_iterator, 3);
  return 1;
}

// Function to get child nodes as an iterator
static int lua_xml_find_child(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  lpcString_t name = luaL_checkstring(L, 2);

  for (xmlNodePtr child = (*node)->children; child; child = child->next) {
    if (!xmlStrcmp(child->name, BAD_CAST name)) {
      xmlNodePtr* value = lua_newuserdata(L, sizeof(xmlNodePtr));
      *value = child;
      luaL_setmetatable(L, API_TYPE_XML_NODE);
      return 1;
    }
  }
  return 0;
}

static int lua_xml_Element(lua_State* L)
{
  xmlNodePtr* node = lua_newuserdata(L, sizeof(xmlNodePtr));
  *node = xmlNewNode(NULL, (xmlChar const*)luaL_checkstring(L, 1));
  luaL_setmetatable(L, API_TYPE_XML_NODE);
  if (lua_type(L, 2) == LUA_TTABLE) {
    lua_pushnil(L);
    while (lua_next(L, -2)) {
      xmlSetProp(*node,
                 XMLSTR(luaL_checkstring(L, -2)),
                 XMLSTR(luaL_checkstring(L, -1)));
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
  }
  return 1;
}

static int lua_xml_SubElement(lua_State* L)
{
  xmlNodePtr* parent = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  xmlNodePtr* child = lua_newuserdata(L, sizeof(xmlNodePtr));
  *child = xmlNewNode(NULL, (xmlChar const*)luaL_checkstring(L, 2));
  luaL_setmetatable(L, API_TYPE_XML_NODE);
  if (lua_type(L, 3) == LUA_TTABLE) {
    lua_pushnil(L);
    while (lua_next(L, 3)) {
      xmlSetProp(*child,
                 XMLSTR(luaL_checkstring(L, -2)),
                 XMLSTR(luaL_checkstring(L, -1)));
      lua_pop(L, 1);
    }
  }
  xmlAddChild(*parent, *child);
  return 1;
}

static int lua_xml_add_child(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  xmlNodePtr* child_node = luaL_checkudata(L, 2, API_TYPE_XML_NODE);
  xmlAddChild(*node, *child_node);
  return 1;
}

static int lua_xml_feee_node(lua_State* L)
{
  //    xmlNodePtr *node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  //    if (!(*node)->parent && !(*node)->doc) {
  //        xmlFreeNode(*node);
  //    }
  //    return 1;
  return 0;
}

static int lua_xml_node_unlink(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  xmlUnlinkNode(*node);
  return 1;
}

static int lua_xml_node_equal(lua_State* L)
{
  xmlNodePtr* a = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  xmlNodePtr* b = luaL_checkudata(L, 2, API_TYPE_XML_NODE);
  lua_pushboolean(L, *a == *b);
  return 1;
}

static void
push_trimmed_string(lua_State* L, lpcString_t str)
{
  SkipSpace(str);
  size_t len = strlen(str);
  while (len > 0 && isspace((unsigned char)str[len - 1]))
    len--; // Trim trailing whitespace
  lua_pushlstring(L, str, len);
}

int lua_xml_node_len(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  lua_Integer len = 0;
  xmlForEach(_, *node) {
    len++;
  }
  lua_pushinteger(L, len);
  return 1;
}

int lua_xml_node_str(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  xmlBufferPtr buf = xmlBufferCreate();
  if (!buf) {
    return lua_error(L);
  }
  xmlNodeDump(buf, (*node)->doc, *node, 0, 1);
  lua_pushstring(L, (lpcString_t)xmlBufferContent(buf));
  xmlBufferFree(buf);
  return 1;
}

int lua_xml_node_index(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  lpcString_t name = luaL_checkstring(L, 2);
  if (!strcmp(name, "tag")) {
    lua_pushstring(L, (lpcString_t)(*node)->name);
    return 1;
  }
  if (!strcmp(name, "text")) {
    if ((*node)->type == XML_PI_NODE) {
      lua_pushstring(L, (lpcString_t)(*node)->content);
      return 1;
    } else if ((*node)->type == XML_TEXT_NODE) {
      //			lua_pushstring(L, (const
      // LPSTR)(*node)->content);
      push_trimmed_string(L, (lpcString_t)(*node)->content);
      return 1;
    } else {
      xmlNode* textBlockNode = *node; // Assume this is the <TextBlock> node
      FOR_EACH_LIST(xmlNode, child , textBlockNode->children) {
        if (child->type == XML_TEXT_NODE) {
          // Get the text content of the text node
          xmlChar* value = child->content;
          lpcString_t str = (lpcString_t)value;
          while (*str && isspace(*str))
            str++;
          lua_pushstring(L, *str ? (lpcString_t)value : "");
          return 1;
        }
      }
      lua_pushstring(L, "");
      return 1;
    }
  }
	if (!strcmp(name, "userdata")) {
		lua_pushinteger(L, (intptr_t)(*node)->_private);
		return 1;
	}
  if (!strcmp(name, "namespace")) {
    if ((*node)->ns) {
      lua_pushstring(L, (lpcString_t)(*node)->ns->href);
    } else {
      lua_pushnil(L);
    }
    return 1;
  }
  if (!strcmp(name, "prefix")) {
    if ((*node)->ns) {
      lua_pushstring(L, (lpcString_t)(*node)->ns->prefix);
    } else {
      lua_pushnil(L);
    }
    return 1;
  }
  if (!strcmp(name, "attributes")) {
    if ((*node)->type == XML_ELEMENT_NODE) {
      lua_pushlightuserdata(L, (*node)->properties);
    } else {
      lua_pushlightuserdata(L, NULL);
    }
    lua_pushcclosure(L, lua_xml_attr_iterator, 1);
    return 1;
  }
  if (!strcmp(name, "nsdef")) {
    lua_pushlightuserdata(L, (*node)->nsDef);
    lua_pushcclosure(L, lua_xml_nsdef_iterator, 1);
    return 1;
  }
  if (!strcmp(name, "findall")) {
    lua_pushcfunction(L, lua_xml_findall);
    return 1;
  }
  if (!strcmp(name, "findallpi")) {
    lua_pushcfunction(L, lua_xml_findallpi);
    return 1;
  }
  if (!strcmp(name, "children")) {
    lua_pushlightuserdata(L, (*node)->children);
    lua_pushcclosure(L, lua_xml_child_iterator, 1);
    return 1;
  }
  if (!strcmp(name, "unfiltered")) {
    lua_pushlightuserdata(L, (*node)->children);
    lua_pushnil(L);
    lua_pushinteger(L, 0);
    lua_pushcclosure(L, lua_xml_child_iterator, 3);
    return 1;
  }
  if (!strcmp(name, "type")) {
    switch ((*node)->type) {
      case XML_ELEMENT_NODE:
        lua_pushstring(L, "element");
        break;
      case XML_PI_NODE:
        lua_pushstring(L, "pi");
        break;
      case XML_TEXT_NODE:
        lua_pushstring(L, "text");
        break;
      case XML_COMMENT_NODE:
        lua_pushstring(L, "comment");
        break;
      default:
        lua_pushstring(L, "unknown");
        break;
    }
    return 1;
  }
  if (!strcmp(name, "parent")) {
    if ((*node)->parent) {
      xmlNodePtr* parent = lua_newuserdata(L, sizeof(xmlNodePtr));
      *parent = (*node)->parent;
      luaL_setmetatable(L, API_TYPE_XML_NODE);
    } else {
      lua_pushnil(L);
    }
    return 1;
  }
  if (!strcmp(name, "empty")) {
    for (xmlNode* child = (*node)->children; child; child = child->next) {
      if (child->type == XML_ELEMENT_NODE) {
        lua_pushboolean(L, FALSE);
        return 1;
      }
    }
    lua_pushboolean(L, TRUE);
    return 1;
  }
  if (!strcmp(name, "get")) {
    lua_pushcfunction(L, lua_xml_get_node_attribute);
    return 1;
  }
  if (!strcmp(name, "set")) {
    lua_pushcfunction(L, lua_xml_set_node_attribute);
    return 1;
  }
  if (!strcmp(name, "find")) {
    lua_pushcfunction(L, lua_xml_find_child);
    return 1;
  }
  if (!strcmp(name, "append")) {
    lua_pushcfunction(L, lua_xml_add_child);
    return 1;
  }
  if (!strcmp(name, "unlink")) {
    lua_pushcfunction(L, lua_xml_node_unlink);
    return 1;
  }
  if (lua_getmetatable(L, 1)) {       // push metatable
    lua_pushvalue(L, 2);              // push key
    lua_rawget(L, -2);                // metatable[key]
    return 1;
  }
  return luaL_error(L, "Incorrect field name %s for %s", name, API_TYPE_XML_NODE);
}

int lua_xml_node_newindex(lua_State* L)
{
  xmlNodePtr* node = luaL_checkudata(L, 1, API_TYPE_XML_NODE);
  lpcString_t name = luaL_checkstring(L, 2);
  if (!strcmp(name, "tag")) {
    lpcString_t value = luaL_checkstring(L, 3);
    xmlNodeSetName(*node, (xmlChar const*)value);
    return 0;
  }
	if (!strcmp(name, "userdata")) {
		(*node)->_private = (void*)(intptr_t)luaL_checkinteger(L, 3);
		return 1;
	}
  if (!strcmp(name, "text")) {
    lpcString_t text = luaL_optstring(L, 3, "");
    for (lpcString_t s = text; *s; s++) {
      if (!isspace(*s)) {
        xmlNodeSetContent(*node, (xmlChar const*)text);
        return 0;
      }
    }
    return 0;
  }
  return luaL_error(
    L, "Incorrect field name %s for %s", name, API_TYPE_XML_NODE);
}

int f_call_xml(lua_State* L);

static int f_xml_read_document(lua_State* L)
{
  lpcString_t module = luaL_checkstring(L, 1);
  xmlDocPtr doc = FS_LoadXML(FS_PathFromModule(module));
  if (!doc || !xmlDocGetRootElement(doc)) {
    return 0;
//  } else if (!xmlStrcmp(xmlDocGetRootElement(doc)->name, XMLSTR("Project"))) {
//    xmlNodePtr root = xmlDocGetRootElement(doc);
//    // local server = require 'orca.backend'
//    lua_getglobal(L, "require");
//    lua_pushstring(L, "orca.backend");
//    lua_pcall(L, 1, 1, 0);
//    // __PEEK = server.peek_message
//    lua_getfield(L, -1, "peek_message"); lua_setglobal(L, "__PEEK");
//    lua_getfield(L, -1, "dispatch_message"); lua_setglobal(L, "__DISPATCH");
//    // local window = server.Window { width = $(ScreenWidth), height = $(ScreenHeight) }
//    lua_getfield(L, -1, "Window");
//    lua_newtable(L);
//    lua_setnumberfield(root, "ScreenWidth", "width", 640);
//    lua_setnumberfield(root, "ScreenHeight", "height", 480);
//    lua_pcall(L, 1, 1, 0);
//    // window:loadPage $(StartupScene)
//    xmlWith(xmlChar, StartupScene, xmlGetProp(root, XMLSTR("StartupScene")), xmlFree) {
//      lua_pushstring(L, (lpcString_t )StartupScene);
//      lua_setglobal(L, "__STARTUP");
//      lua_getfield(L, -1, "loadPage");
//      lua_pushvalue(L, -2);
//      lua_pushstring(L, (lpcString_t )StartupScene);
//      if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
//        Con_Error("%s", lua_tostring(L, -1));
//        return 1;
//      }
//    }
//    xmlFreeDoc(doc);
//    return luaL_dostring(L, run_loop);
  } else {
    xmlDocPtr* ptr = lua_newuserdata(L, sizeof(xmlDocPtr*));
    luaL_setmetatable(L, "XmlDoc");
    *ptr = doc;
    return 1;
  }
}

static int f_find_xml(lua_State* L)
{
  lpcString_t module = luaL_checkstring(L, 1);
  path_t path = { 0 };
  snprintf(path, sizeof(path), "%s.xml", FS_PathFromModule(module));
  if (FS_FileExists(path)) {
    lua_pushcfunction(L, f_xml_read_document);
    return 1;
  } else {
    return 0;
  }
}

lua_State* global_L = NULL;

ORCA_API int luaopen_orca_parsers_xml(lua_State* L)
{
  luaL_newlib(L,
              ((luaL_Reg[]){ { "load", lua_xml_read_document },
                             { "parse", lua_xml_parse_document },
                             { "new", lua_xml_new_document },
                             { "Element", lua_xml_Element },
                             { "SubElement", lua_xml_SubElement },
                             { NULL, NULL } }));

  luaL_newmetatable(L, API_TYPE_XML_NODE);
  luaL_setfuncs(L,
                ((luaL_Reg[]){ { "__gc", lua_xml_feee_node },
                               { "__eq", lua_xml_node_equal },
                               { "__index", lua_xml_node_index },
                               { "__newindex", lua_xml_node_newindex },
                               { "__len", lua_xml_node_len },
                               { "__tostring", lua_xml_node_str },
                               { NULL, NULL } }),
                0);
  lua_setfield(L, -2, "XmlElement");
  
  luaL_newmetatable(L, API_TYPE_XML_DOC);
  luaL_setfuncs(L,
                ((luaL_Reg[]){ { "__gc", lua_xml_free_document },
                               { "__index", lua_xml_doc_index },
                               { "__newindex", lua_xml_doc_newindex },
                               { "__tostring", lua_xml_doc_str },
                               { "__call", f_call_xml },
                               { NULL, NULL } }),
                0);
  lua_setfield(L, -2, "XmlDoc");

  lua_register(L, "fs_findxml", f_find_xml);
  luaL_dostring(L, "table.insert(package.searchers, fs_findxml)");
  
  extern lpcString_t
  (*_PDESC_Parse)(lpObject_t hobj,
                  lpcPropertyDesc_t pdesc,
                  lpProperty_t property,
                  lpcString_t string,
                  void* dest);

  extern lpObject_t
  (*_OBJ_LoadDocument)(lua_State* L, xmlDocPtr doc);

  _PDESC_Parse = PDESC_Parse;
  _OBJ_LoadDocument = OBJ_LoadDocument;
  
  extern shortStr_t tags[MAX_TAGS];
  memset(tags, 0, sizeof(tags));
  
  API_CallRequire(L, "orca.filesystem", 1);
  lua_pushcfunction(L, f_loadProject);
  lua_setfield(L, -2, "loadBundle");
  lua_pop(L, 1);
  
  global_L = L;
  
  return 1;
}
