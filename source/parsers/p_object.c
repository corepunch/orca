#include "xml_local.h"

#include <libxml/uri.h>

#include <include/api.h>
#include <source/filesystem/filesystem.h>

// #define SHOW_UNDEFINES

extern lpObject_t root_node;

BINDING_MODE
xmlParseBindingMode(xmlChar const* s)
{
  if (!xmlStrcmp(s, XMLSTR("expression")))
    return BIND_EXPRESSION;
  if (!xmlStrcmp(s, XMLSTR("oneway")))
    return BING_ONEWAY;
  return BIND_EXPRESSION;
}

enum property_attribute
xmlParsePropertyAttr(xmlChar const* s)
{
  if (s == NULL)
    return ATTR_WHOLE_PROPERTY;
  if (!xmlStrcmp(s, XMLSTR("WHOLE_PROPERTY")))
    return ATTR_WHOLE_PROPERTY;
  if (!xmlStrcmp(s, XMLSTR("COLOR_R")))
    return ATTR_COLOR_R;
  if (!xmlStrcmp(s, XMLSTR("COLOR_G")))
    return ATTR_COLOR_G;
  if (!xmlStrcmp(s, XMLSTR("COLOR_B")))
    return ATTR_COLOR_B;
  if (!xmlStrcmp(s, XMLSTR("COLOR_A")))
    return ATTR_COLOR_A;
  if (!xmlStrcmp(s, XMLSTR("VECTOR_X")))
    return ATTR_VECTOR_X;
  if (!xmlStrcmp(s, XMLSTR("VECTOR_Y")))
    return ATTR_VECTOR_Y;
  if (!xmlStrcmp(s, XMLSTR("VECTOR_Z")))
    return ATTR_VECTOR_Z;
  if (!xmlStrcmp(s, XMLSTR("VECTOR_W")))
    return ATTR_VECTOR_W;
  assert(0);
  return ATTR_WHOLE_PROPERTY;
}

static void
XML_ParsePropertyNode(lua_State* L, xmlNodePtr it, lpObject_t object)
{
  bool_t bEnabled = TRUE;
  enum property_attribute eAttr = ATTR_WHOLE_PROPERTY;
  BINDING_MODE eMode = BIND_EXPRESSION;
  lpProperty_t property = NULL;

  xmlWith(xmlChar, Value, xmlGetProp(it, XMLSTR("Value")), xmlFree) {
    PROP_Parse2(L, object, (lpcString_t)it->name, (lpcString_t)Value);
    xmlFree(Value);
    return;
  }
  if (FAILED(OBJ_FindLongProperty(object, (LPSTR)it->name, &property))) {
#ifdef SUPPORT_DYNAMIC_PROPERTIES
    if (!xmlHasProp(it, XMLSTR("Type"))) {
      property = PROP_Create(L, object, (LPSTR)it->name, kDataTypeNone, NULL);
    } else xmlWith(xmlChar, Type, xmlGetProp(it, XMLSTR("Type")), xmlFree) {
      property = PROP_CreateFromString(L, object, (LPSTR)it->name, (LPSTR)Type);
    }
#else
    Con_Error("Can not instantiate child node `%s` in %s(%s)", it->name, OBJ_GetName(object), OBJ_GetClassName(object));
    return;
#endif
  }
  xmlWith(xmlChar, Enabled, xmlGetProp(it, XMLSTR("Enabled")), xmlFree) {
    bEnabled = xmlStrcmp(Enabled, XMLSTR("true")) == 0;
  }
  xmlWith(xmlChar, Attribute, xmlGetProp(it, XMLSTR("Attribute")), xmlFree) {
    eAttr = xmlParsePropertyAttr(Attribute);
  }
  xmlWith(xmlChar, Mode, xmlGetProp(it, XMLSTR("Mode")), xmlFree) {
    eMode = xmlParseBindingMode(Mode);
  }
  xmlWith(xmlChar, Content, xmlNodeGetContent(it), xmlFree) {
    lpcString_t source = (lpcString_t)Content;
    struct token *compiled = bEnabled?Token_Create(source):NULL;
    PROP_AttachProgram(property, eAttr, compiled, source);
  }
}

//static struct component*
//_LoadComponent(lua_State* L, xmlNodePtr xml, lpObject_t hobj)
//{
//  struct component* trigger = NULL;
//  xmlWith(xmlChar, event, xmlGetProp(xml, XMLSTR("Event")), xmlFree) {
//    lua_getfield(L, LUA_REGISTRYINDEX, (LPSTR)event);
//    lua_getfield(L, -1, "__nativeclass");
//    trigger = OBJ_AddComponent(hobj, lua_touserdata(L, -1));
//    lua_pop(L, 2);
//  }
//  if (!trigger)
//    return NULL;
//  FOR_EACH_LIST(xmlAttr, attr, xml->properties) {
//    if (!xmlStrcmp(attr->name, XMLSTR("Event")))
//      continue;
//    lpProperty_t hprop = CMP_CreateProperty2(L, trigger, (lpcString_t)attr->name);
//    if (!hprop) {
//      Con_Error("Can't create property %s in %s",
//                 (lpcString_t)attr->name,
//                 (lpcString_t)xml->name);
//      continue;
//    }
//    xmlWith(xmlChar, value, xmlNodeGetContent((xmlNode*)attr), xmlFree) {
//      PROP_Parse(hprop, (lpcString_t)value);
//    }
//  }
//  return trigger;
//}

//static void
//_LoadActions(lua_State* L,
//             xmlNodePtr xml,
//             lpObject_t hobj,
//             struct component* trigger)
//{
//  struct component* action;
//  lpProperty_t prop;
//  xmlForEach(it, xml)
//  {
//    if ((action = _LoadComponent(L, it, hobj))) {
//      if ((prop = CMP_CreateProperty2(L, action, "Trigger"))) {
//        PROP_SetValue(prop, &trigger);
//      }
//    }
//  }
//}

static void
XML_ParseStyleSheet(lua_State* L, lpObject_t hobj, lpcString_t filename)
{
  bool_t loaded = FALSE;
  xmlWith(xmlDoc, doc, FS_LoadXML(filename), xmlFreeDoc)
  {
    xmlNodePtr root = xmlDocGetRootElement(doc);
    loaded = TRUE;
    if (xmlStrcmp(root->name, XMLSTR("Styles"))) {
      Con_Error("%s: expected Styles root element", filename);
      break;
    }
    xmlForEach(it, xmlDocGetRootElement(doc))
    {
      if (xmlStrcmp(it->name, XMLSTR("Style")))
        continue;
      xmlWith(xmlChar, id, xmlGetProp(it, XMLSTR("Key")), xmlFree)
      {
        FOR_EACH_LIST(xmlAttr, attr, it->properties)
        {
          if (!xmlStrcmp(attr->name, XMLSTR("Key")) ||
              !xmlStrcmp(attr->name, XMLSTR("StyleTargetType")) ||
              !xmlStrcmp(attr->name, XMLSTR("StyleType")))
            continue;
          xmlWith(xmlChar, value, xmlNodeGetContent((xmlNode*)attr), xmlFree)
          {
            OBJ_AddStyleClass(hobj,(lpcString_t)id,(lpcString_t)attr->name,(lpcString_t)value,0);
          }
        }
      }
    }
  }
  if (!loaded) {
    Con_Error("%s: stylesheet not found", filename);
  }
}

bool_t _ShallSkipNode(xmlNodePtr xml) {
  bool_t bSkip = FALSE;
  
  xmlWith(xmlChar, IsDisabled, xmlGetProp(xml, XMLSTR("IsDisabled")), xmlFree) {
    bSkip = !xmlStrcmp(IsDisabled, XMLSTR("true"));
  }
  
  if (bSkip ||
      !xmlStrcmp(XMLSTR("AnimationPlayer"), xml->name) ||
      !xmlStrcmp(XMLSTR("ValueTicker"), xml->name))
  {
    return TRUE;
  } else {
    return FALSE;
  }
}

ORCA_API void
OBJ_RegisterPrefab(lpObject_t hobj, lpcString_t szFileName);

void FS_RegisterObject(lpObject_t object, lpcString_t path) {
  if (GetProject(object)) {
    OBJ_AddChild(FS_GetWorkspace(), object, FALSE);
  } else {
    if (!strrchr(path, '/')) {
      Con_Error("Expected / in %s", path);
      return;
    }
    path_t tmp = {0}, dir = {0};
    strncpy(tmp, path, MIN(strrchr(path, '/')-path, sizeof(tmp)));
    lpObject_t library = OBJ_FindByPath(FS_GetWorkspace(), tmp);
    FS_GetDirName(tmp, dir, sizeof(dir));
    if (!library && OBJ_FindChild(FS_GetWorkspace(), dir, FALSE)) {
      Con_Error("Could not find library %s, creating", tmp);
      xmlWith(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFreeDoc) {
        xmlNodePtr root = xmlNewNode(NULL, XMLSTR("Library"));
        xmlSetProp(root, XMLSTR("Name"), XMLSTR(FS_GetBaseName(tmp)));
        xmlDocSetRootElement(doc, root);
        extern lua_State *global_L;
        if ((library = OBJ_LoadDocument(global_L, doc))) {
          OBJ_AddChild(OBJ_FindChild(FS_GetWorkspace(), dir, FALSE), library, FALSE);
        }
      }
    }
    if (library) {
      OBJ_AddChild(library, object, FALSE);
    } else{
      Con_Error("Could not find or create library %s", tmp);
    }
  }
}

ORCA_API lpObject_t
XML_ParseObjectNode(lua_State* L, xmlNodePtr xml, lpObject_t root, xmlDocPtr doc)
{
  lpcString_t szClass = (lpcString_t)xml->name;
  lpObject_t hobj = NULL;
  lpObject_t childobj = NULL;

  xmlWith(xmlChar, Width, xmlGetProp(xml, XMLSTR("Width")), xmlFree) {
    extern int ScreenWidth;
    ScreenWidth = atoi((lpcString_t)Width);
  }
  xmlWith(xmlChar, Height, xmlGetProp(xml, XMLSTR("Height")), xmlFree) {
    extern int ScreenHeight;
    ScreenHeight = atoi((lpcString_t)Height);
  }

  if (!strcmp(szClass, "LayerPrefabPlaceholder") ||
      !strcmp(szClass, "ObjectPrefabPlaceholder"))
  {
    xmlWith(xmlChar,tmp,xmlGetProp(xml, XMLSTR("PlaceholderTemplate")),xmlFree) {
      xmlWith(xmlDoc, doc, FS_LoadXML((lpcString_t)tmp), xmlFreeDoc) {
        hobj = OBJ_LoadDocument(L, doc);
      }
      if (!hobj) { // TODO: somehow it messes with hierarchy, so try xml first
        lua_getglobal(L, "require");
        lua_pushstring(L, (const char*)tmp);
        if (lua_pcall(L, 1, 1, 0) || lua_pcall(L, 0, 1, 0)) {
          Con_Error("%s", lua_tostring(L, -1));
          lua_pop(L, 1);
          return NULL;
        } else {
          hobj = luaX_checkObject(L, -1);
        }
      }
    }
    if (!hobj) {
      return NULL;
    } else {
      OBJ_SetFlags(hobj, OBJ_GetFlags(hobj)|OF_LOADED_FROM_PREFAB);
    }
  } else {
    lua_getfield(L, LUA_REGISTRYINDEX, szClass);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
//      luaL_error(L, "Class %s not found", szClass);
      return NULL;
    }
    if (lua_type(L, -1) == LUA_TSTRING) {
      luaL_error(L, "LoadXML(): %s", lua_tostring(L, -1));
      return NULL;
    }
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
      luaL_error(L, "LoadXML(): %s", lua_tostring(L, -1));
      return NULL;
    }
    hobj = luaX_checkObject(L, -1);
  }
  
  OBJ_SetName(hobj, szClass);
  OBJ_SetClassName(hobj, (lpcString_t )xml->name);

  if (doc && doc->URL) {
    xmlWith(char, decoded, xmlURIUnescapeString((char*)doc->URL, -1, NULL), xmlFree) {
      OBJ_RegisterPrefab(hobj, decoded);
      FS_RegisterObject(hobj, decoded);
    }
  }
  
  xmlWith(xmlChar, Name, xmlGetProp(xml, XMLSTR("Name")), xmlFree) {
    OBJ_SetName(hobj, (lpcString_t)Name);
  }

  root_node = root ? root : hobj;

  FOR_EACH_LIST(xmlAttr, attr, xml->properties)
  {
    if (!xmlStrcmp(attr->name, XMLSTR("PlaceholderTemplate")))
      continue;
    xmlWith(xmlChar, value, xmlNodeGetContent((xmlNode*)attr), xmlFree)
    {
      root_node = root ? root : hobj;
      int top = lua_gettop(L);
      PROP_Parse2(L, hobj, (lpcString_t)attr->name, (lpcString_t)value);
      lua_pop(L, lua_gettop(L) - top);
    }
  }
  
  if (xml->children &&
      !xml->children->next &&
      xml->children->type==XML_TEXT_NODE)
  {
    OBJ_SetTextContent(hobj, (char*)xml->children->content);
  }
  
  FOR_EACH_LIST(xmlNode, it, (xml)->children)
  switch (it->type) {
    case XML_ELEMENT_NODE:
      root_node = root ? root : hobj;
      if (!xmlStrcmp(it->name, XMLSTR("script"))) {
        xmlWith(xmlChar, content, xmlNodeGetContent(it), xmlFree) {
          if (luaL_loadstring(L, (lpcString_t )content) != LUA_OK) {
            lpcString_t err = lua_tostring(L, -1);
            Con_Error("Load error: %s", err);
            lua_pop(L, 1);  // remove error
          }
//          lua_pushvalue(L, -2);             // copy env table
          luaX_pushObject(L, hobj);
          lua_setupvalue(L, -2, 1);         // set _ENV as first upvalue of the loaded chunk
          if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            Con_Error("Runtime error: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
          }
          lua_pop(L, 1);
        }
      } else if (!xmlStrcmp(it->name, XMLSTR("Animation"))) {
        xmlWith(xmlChar, Clip, xmlGetProp(it, XMLSTR("Clip")), xmlFree) {
          xmlWith(xmlDoc, doc, FS_LoadXML((lpcString_t)Clip), xmlFreeDoc) {
            lpKeyframeAnim_t anim = ANIM_Load(doc);
            if (anim) {
              xmlWith(xmlChar, id, xmlGetProp(it, XMLSTR("Key")), xmlFree) {
                anim->name = strdup((lpcString_t )id);
              }
              OBJ_AddAnimation(hobj, anim);
            }
          }
        }
      } else if (!xmlStrcmp(it->name, XMLSTR("EventListener"))) {
//        xmlWith(struct component, trigger, _LoadComponent(L, it, hobj), _keep) {
//          _LoadActions(L, it, hobj, trigger);
//        }
      } else if (!xmlStrcmp(it->name, XMLSTR("Entry"))) {
        xmlWith(xmlChar, id, xmlGetProp(it, XMLSTR("Key")), xmlFree) {
          xmlWith(xmlChar, content, xmlNodeGetContent(it), xmlFree) {
            OBJ_AddAlias(hobj, (lpcString_t)id, (lpcString_t)content);
          }
        }
      } else if (!xmlStrcmp(it->name, XMLSTR("ResourceDictionaryItem"))) {
        xmlForEach(entry, it) {
          xmlWith(xmlChar, id, xmlGetProp(entry, XMLSTR("Key")), xmlFree) {
            xmlWith(xmlChar, content, xmlNodeGetContent(entry), xmlFree) {
              OBJ_AddAlias(hobj, (lpcString_t)id, (lpcString_t)content);
            }
          }
        }
      } else if (!xmlStrcmp(it->name, XMLSTR("StyleSheet"))) {
        xmlWith(xmlChar, content, xmlNodeGetContent(it), xmlFree) {
          XML_ParseStyleSheet(L, hobj, (lpcString_t )content);
        }
      } else if (_ShallSkipNode(it)) {
        // skip
      } else if ((childobj = XML_ParseObjectNode(L, it, root ? root : hobj, NULL))) {
        OBJ_AddChild(hobj, childobj, FALSE);
        lua_pop(L, 1);
      } else {
        XML_ParsePropertyNode(L, it, hobj);
      }
      break;
//    case XML_PI_NODE:
//      if (luaL_loadstring(L, (lpcString_t )it->content) != LUA_OK) {
//        lpcString_t err = lua_tostring(L, -1);
//        Con_Error("Load error: %s", err);
//        lua_pop(L, 1);  // remove error
//      }
//      
//      lua_pushvalue(L, -2);             // copy env table
//      lua_setupvalue(L, -2, 1);         // set _ENV as first upvalue of the loaded chunk
//      
//      if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
//        Con_Error("Runtime error: %s", lua_tostring(L, -1));
//        lua_pop(L, 1);
//      }
//      break;
    default:
      break;
  }
  
  OBJ_SendMessageW(hobj, kEventStart, 0, NULL);
  
  return hobj;
}

#ifdef USE_PI_NODES
static LPSTR
extract_attribute(LPSTR input, lpcString_t attr)
{
  for (LPSTR s = strstr(input, attr); s; s = NULL) {
    for (LPSTR end = strchr(s + strlen(attr), '"'); end; end = NULL) {
      *end = 0;
      return s + strlen(attr);
    }
  }
  return NULL;
}
#endif
//	for e in xhtml:findallpi('export') do
//		local ename = e.content:match('name="([^"]+)"')
//		local ecode = string.format("{##Template/%s}", ename)
//		local eprop = element:findOrCreateProperty(ename)
//		Element.add_binding(eprop, nil, Element.compile_program(ecode),
// nil) 		end

ORCA_API lpObject_t
OBJ_LoadDocument(lua_State* L, xmlDocPtr doc)
{
  lpObject_t hobj = XML_ParseObjectNode(L, xmlDocGetRootElement(doc), NULL, doc);
#ifdef USE_PI_NODES
//  for (xmlNodePtr n = xmlDocGetRootElement(doc)->prev; n; n = n->prev) {
//    if (n->type != XML_PI_NODE)
//      continue;
//    if (!xmlStrcmp(n->name, XMLSTR("xml-stylesheet"))) {
//      xmlWith(xmlChar, content, xmlNodeGetContent(n), xmlFree) {
//        lpcString_t type = extract_attribute((LPSTR)content, "type=\"");
//        if (!strcmp(type, "text/xml")) {
//          LPSTR href = extract_attribute((LPSTR)content, "href=\"");
//          for (LPSTR s = strstr(href, ".xml"); s; *s = 0, s = NULL);
//          _ParseStyleSheet(L, hobj, href);
//        }
//      }
//    }
//  }
#endif
  return hobj;
}

ORCA_API int f_call_xml(lua_State* L)
{
  xmlDocPtr* doc = luaL_checkudata(L, 1, "XmlDoc");
  lpObject_t hobj = OBJ_LoadDocument(L, *doc);
  if (hobj) {
    OBJ_AssignAliases(hobj, (*doc)->name);
    luaX_pushObject(L, hobj);
    return 1;
  } else {
    return luaL_error(L, "Can't instantiate document %s", (*doc)->name);
  }
}

// static int get_changed_property_name(lua_State* L) {
//	lpcString_t  str = luaL_checkstring(L, 1);
//	lpcString_t  suffix = "Changed";
//
//	size_t str_len = strlen(str);
//	size_t suffix_len = 7;//strlen(suffix);
//
//	if (str_len >= suffix_len) {
//		if (strcmp(str + str_len - suffix_len, suffix) == 0) {
//			size_t property_len = str_len - suffix_len;
//			lua_pushlstring(L, str, property_len);
//			return 1;
//		}
//	}
//	lua_pushnil(L);
//	return 1;
// }
//
