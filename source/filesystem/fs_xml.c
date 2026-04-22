#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <source/core/core_properties.h>

// Forward declarations
static lpObject_t FS_ConstructNode(lua_State* L, xmlNodePtr element);
int f_xml_module_loader(lua_State* L);

// External declaration of the property string parser defined in geometry/flow.c
extern int parse_property(lua_State* L, const char* str,
                           struct PropertyType const* prop, void* valueptr);

// External declaration of luaX_readProperty (reads a Lua value at idx into property)
extern int luaX_readProperty(lua_State* L, int idx, lpProperty_t p);

// Map a PropertyAttribute name string to its enum value.
static enum PropertyAttribute
_ParseAttributeStr(lpcString_t s)
{
  static lpcString_t names[] = {
    "WholeProperty", "ColorR", "ColorG", "ColorB", "ColorA",
    "VectorX", "VectorY", "VectorZ", "VectorW", NULL
  };
  for (int i = 0; names[i]; i++) {
    if (!strcmp(s, names[i])) return (enum PropertyAttribute)i;
  }
  return kPropertyAttributeWholeProperty;
}

// Map a BindingMode name string to its enum value.
static enum BindingMode
_ParseBindingModeStr(lpcString_t s)
{
  static lpcString_t names[] = {
    "OneWay", "TwoWay", "OneWayToSource", "Expression", NULL
  };
  for (int i = 0; names[i]; i++) {
    if (!strcmp(s, names[i])) return (enum BindingMode)i;
  }
  return kBindingModeExpression;
}

// Parse a single XML attribute value and set it on obj.
// Handles Name/id, class, and all registered properties.
static void
_SetPropertyFromString(lua_State* L, lpObject_t obj, lpcString_t name, lpcString_t value)
{
  // Skip editor-only / template attributes
  if (!strcmp(name, "ClassName") || !strcmp(name, "PlaceholderTemplate"))
    return;

  // Identity attributes
  if (!strcmp(name, "Name") || !strcmp(name, "id")) {
    OBJ_SetName(obj, value);
    return;
  }

  // CSS class list
  if (!strcmp(name, "class")) {
    _SendMessage(obj, StyleController, AddClasses, .ClassNames = value);
    return;
  }

  // Look up the property type descriptor (implicit first, then full-qualified)
  lpcPropertyType_t pdesc = OBJ_FindImplicitProperty(obj, name);
  if (!pdesc) pdesc = OBJ_FindExplicitProperty(obj, name);
  if (!pdesc) {
    Con_Printf("fs_xml: Unknown property '%s' for class '%s'",
               name, OBJ_GetClassName(obj));
    return;
  }

  // Find or create the property slot
  lpProperty_t prop = NULL;
  if (FAILED(OBJ_FindShortProperty(obj, pdesc->Name, &prop))) {
    Con_Printf("fs_xml: Could not get property slot for '%s'", pdesc->Name);
    return;
  }

  // Parse the string value into a stack-allocated buffer.
  // MAX_PROPERTY_STRING (256) is large enough for any scalar type.
  char tmpbuf[MAX_PROPERTY_STRING] = {0};
  if (!parse_property(L, value, pdesc, tmpbuf))
    return;

  PROP_SetValue(prop, tmpbuf);

  // parse_property allocates a new string with strdup() for kDataTypeString.
  // PROP_SetValue → PROP_SetStateValue then strdup's it again internally, so
  // free the temporary copy here to avoid a memory leak.
  if (pdesc->DataType == kDataTypeString) {
    free(*(char**)tmpbuf);
  }
}

// Build an object-property child element (e.g. <Grid.Columns> or binding).
static void
_ConstructProperty(lua_State* L, lpObject_t obj,
                   lpcPropertyType_t pdesc, xmlNodePtr element)
{
  if (pdesc->IsArray && L) {
    // Array properties: build a Lua table where each entry is a table of
    // string-valued attributes, then use luaX_readProperty which handles
    // array construction via the registered struct metatable.
    lpProperty_t prop = NULL;
    if (FAILED(OBJ_FindShortProperty(obj, pdesc->Name, &prop))) {
      Con_Printf("fs_xml: Could not get array property slot '%s'", pdesc->Name);
      return;
    }

    lua_newtable(L);
    int table_idx = lua_gettop(L);
    int count = 0;

    xmlFindAll(child, element, XMLSTR(pdesc->TypeString)) {
      lua_newtable(L);
      FOR_EACH_LIST(xmlAttr, attr, child->properties) {
        xmlChar* val = xmlGetProp(child, attr->name);
        if (val) {
          lua_pushstring(L, (lpcString_t)val);
          lua_setfield(L, -2, (lpcString_t)attr->name);
          xmlFree(val);
        }
      }
      lua_rawseti(L, table_idx, ++count);
    }

    luaX_readProperty(L, table_idx, prop);
    lua_pop(L, 1); // pop the table
    return;
  }

  // Non-array case: check for a Value attribute first
  xmlChar* val = xmlGetProp(element, XMLSTR("Value"));
  if (val) {
    _SetPropertyFromString(L, obj, pdesc->Name, (lpcString_t)val);
    xmlFree(val);
    return;
  }

  // No Value attribute: treat text content as a binding expression
  xmlChar* text = xmlNodeGetContent(element);
  if (text && *text) {
    enum PropertyAttribute attribute = kPropertyAttributeWholeProperty;
    enum BindingMode mode = kBindingModeExpression;
    bool_t enabled = TRUE;

    xmlChar* attr_xml = xmlGetProp(element, XMLSTR("Attribute"));
    xmlChar* mode_xml = xmlGetProp(element, XMLSTR("Mode"));
    xmlChar* en_xml   = xmlGetProp(element, XMLSTR("Enabled"));

    if (attr_xml) { attribute = _ParseAttributeStr((lpcString_t)attr_xml); xmlFree(attr_xml); }
    if (mode_xml) { mode      = _ParseBindingModeStr((lpcString_t)mode_xml); xmlFree(mode_xml); }
    if (en_xml)   { enabled   = strcmp((lpcString_t)en_xml, "false") != 0;   xmlFree(en_xml); }

    OBJ_AttachPropertyProgram(obj, pdesc->Name, (lpcString_t)text,
                               attribute, mode, enabled);
  }
  if (text) xmlFree(text);
}

// Handle <AnimationPlayer ...> child: attach-only component whose attributes
// are parsed as properties of the owning object.
static void
_HandleAnimationPlayer(lua_State* L, lpObject_t obj, xmlNodePtr element)
{
  OBJ_AddComponent(obj, ID_AnimationPlayer);
  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(L, obj, (lpcString_t)attr->name, (lpcString_t)val);
      xmlFree(val);
    }
  }
}

// Handle <AnimationCurve ...> child: regular child object of an AnimationClip.
static void
_HandleAnimationCurve(lua_State* L, lpObject_t obj, xmlNodePtr element)
{
  lpcClassDesc_t cls = OBJ_FindClass("AnimationCurve");
  if (!cls) {
    Con_Printf("fs_xml: AnimationCurve class not found");
    return;
  }
  lpObject_t curve = OBJ_Create(L, cls);

  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(L, curve, (lpcString_t)attr->name, (lpcString_t)val);
      xmlFree(val);
    }
  }

  // Process explicit-property child elements (e.g. <AnimationCurve.Keyframes>)
  xmlForEach(sub, element) {
    lpcPropertyType_t pdesc = OBJ_FindExplicitProperty(curve, (lpcString_t)sub->name);
    if (pdesc) {
      _ConstructProperty(L, curve, pdesc, sub);
    }
  }

  OBJ_AddChild(obj, curve, FALSE);
  OBJ_SendMessageW(curve, ID_Object_Start, 0, NULL);
}

// Handle <StateManagerController ...> child: attach-only component with an
// optional inline <StateManager> child element.
static void
_HandleStateManagerController(lua_State* L, lpObject_t obj, xmlNodePtr element)
{
  OBJ_AddComponent(obj, ID_StateManagerController);

  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(L, obj, (lpcString_t)attr->name, (lpcString_t)val);
      xmlFree(val);
    }
  }

  // Look for an inline <StateManager> child element
  xmlFindAll(sub, element, XMLSTR("StateManager")) {
    lpObject_t sm = FS_ConstructNode(L, sub);
    if (sm) {
      lpProperty_t prop = NULL;
      if (SUCCEEDED(OBJ_FindShortProperty(obj, "StateManager", &prop))) {
        PROP_SetValue(prop, &sm);
      }
    }
    break; // only the first <StateManager> is used
  }
}

// Execute an inline <script> element in a Lua environment where `self` is obj.
static void
_RunScript(lua_State* L, lpObject_t obj, xmlNodePtr element)
{
  if (!L) return;
  xmlChar* text = xmlNodeGetContent(element);
  if (!text || !*text) {
    if (text) xmlFree(text);
    return;
  }

  lpcString_t src = (lpcString_t)text;

  if (luaL_loadstring(L, src) != LUA_OK) {
    Con_Printf("fs_xml: script load error: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    xmlFree(text);
    return;
  }

  // Build environment table: { self = obj } with _G as fallback via __index
  lua_newtable(L);                  // env
  luaX_pushObject(L, obj);
  lua_setfield(L, -2, "self");      // env.self = obj

  lua_newtable(L);                  // env_mt
  lua_pushglobaltable(L);
  lua_setfield(L, -2, "__index");   // env_mt.__index = _G
  lua_setmetatable(L, -2);          // setmetatable(env, env_mt)

  // Set the loaded chunk's _ENV upvalue to our custom environment
  lua_setupvalue(L, -2, 1);         // chunk._ENV = env; pops env

  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    Con_Printf("fs_xml: script error: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
  }

  xmlFree(text);
}

// Build an Object tree from an XML element node.
static lpObject_t
FS_ConstructNode(lua_State* L, xmlNodePtr element)
{
  lpcClassDesc_t cls = OBJ_FindClass((lpcString_t)element->name);
  if (!cls) {
    Con_Printf("fs_xml: Unknown element type '%s'", (lpcString_t)element->name);
    return NULL;
  }

  lpObject_t obj = OBJ_Create(L, cls);

  // Parse all XML attributes as object properties
  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(L, obj, (lpcString_t)attr->name, (lpcString_t)val);
      xmlFree(val);
    }
  }

  // If the element has a direct text child (e.g. <Label>Hello</Label>), store it
  // and return early without processing child elements.  The XML parser has
  // XML_PARSE_NOBLANKS active, so only non-blank text nodes survive.
  xmlChar* direct_text = NULL;
  FOR_EACH_LIST(xmlNode, tnode, element->children) {
    if (tnode->type == XML_TEXT_NODE && xmlStrlen(tnode->content) > 0) {
      direct_text = tnode->content;
      break;
    }
  }
  if (direct_text) {
    OBJ_SetTextContent(obj, (lpcString_t)direct_text);
    OBJ_SendMessageW(obj, ID_Object_Start, 0, NULL);
    return obj;
  }

  // Process child elements
  xmlForEach(sub, element) {
    // Skip elements marked as disabled
    xmlChar* dis = xmlGetProp(sub, XMLSTR("IsDisabled"));
    bool_t is_disabled = dis && !xmlStrcmp(dis, XMLSTR("true"));
    if (dis) xmlFree(dis);
    if (is_disabled) continue;

    lpcString_t tag = (lpcString_t)sub->name;

    // Explicit property child (e.g. <Grid.Columns> or <AnimationClip.Keyframes>)
    lpcPropertyType_t pdesc = OBJ_FindExplicitProperty(obj, tag);
    if (pdesc) {
      _ConstructProperty(L, obj, pdesc, sub);
      continue;
    }

    // Special attach-only components and Lua-only elements
    if (!strcmp(tag, "AnimationPlayer")) {
      _HandleAnimationPlayer(L, obj, sub);
    } else if (!strcmp(tag, "AnimationCurve")) {
      _HandleAnimationCurve(L, obj, sub);
    } else if (!strcmp(tag, "StateManagerController")) {
      _HandleStateManagerController(L, obj, sub);
    } else if (!strcmp(tag, "script")) {
      _RunScript(L, obj, sub);
    } else if (!strcmp(tag, "EventListener") ||
               !strcmp(tag, "StyleSheet")    ||
               !strcmp(tag, "ValueTicker")) {
      // No-ops for legacy Lua-only elements
    } else {
      // Regular child object: recurse and attach
      lpObject_t child = FS_ConstructNode(L, sub);
      if (child) {
        OBJ_AddChild(obj, child, FALSE);
      }
    }
  }

  OBJ_SendMessageW(obj, ID_Object_Start, 0, NULL);
  return obj;
}

// Lua-callable function: fs_loadxmlmodule(module_name) → loader | nil
// Used as a package.searchers entry to allow require("foo") to load foo.xml.
int f_find_xml_module(lua_State* L)
{
  lpcString_t module = luaL_checkstring(L, 1);
  path_t path = {0};
  snprintf(path, sizeof(path), "%s.xml", FS_PathFromModule(module));

  // Check virtual filesystem first, then disk
  struct file* fp = FS_LoadFile(path);
  if (fp) {
    FS_FreeFile(fp);
  } else {
    // Check directly on disk (for absolute/CWD-relative paths)
    FILE* f = fopen(path, "rb");
    if (!f) return 0; // not found
    fclose(f);
  }

  // Return a loader closure that captures the resolved path
  lua_pushstring(L, path);
  lua_pushstring(L, module);
  lua_pushcclosure(L, f_xml_module_loader, 2);
  return 1;
}

// Loader closure: called by Lua require() machinery after the searcher returned it.
// upvalue 1 = file path (string), upvalue 2 = module name (string)
int f_xml_module_loader(lua_State* L)
{
  lpcString_t path        = luaL_checkstring(L, lua_upvalueindex(1));
  lpcString_t module_name = luaL_checkstring(L, lua_upvalueindex(2));

  lpObject_t obj = FS_LoadObjectFromXML(L, path);
  if (!obj) {
    return luaL_error(L, "fs_xml: Failed to load XML file: %s", path);
  }
  OBJ_SetSourceFile(obj, module_name);
  luaX_pushObject(L, obj);
  return 1;
}

// --- Public API -----------------------------------------------------------

ORCA_API lpObject_t
FS_LoadObjectFromXML(lua_State* L, lpcString_t path)
{
  struct file* fp = FS_LoadFile(path);
  if (fp) {
    xmlDoc* doc = xmlReadMemory((lpcString_t)fp->data, (int)fp->size,
                                path, NULL, XML_FLAGS);
    FS_FreeFile(fp);
    if (!doc) {
      Con_Printf("FS_LoadObjectFromXML: failed to parse '%s'", path);
      return NULL;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    lpObject_t result = root ? FS_ConstructNode(L, root) : NULL;
    xmlFreeDoc(doc);
    return result;
  }

  // Fall back to reading directly from disk (for test/tool use)
  xmlDoc* doc = xmlReadFile(path, NULL, XML_FLAGS);
  if (!doc) {
    Con_Printf("FS_LoadObjectFromXML: failed to read '%s'", path);
    return NULL;
  }
  xmlNodePtr root = xmlDocGetRootElement(doc);
  lpObject_t result = root ? FS_ConstructNode(L, root) : NULL;
  xmlFreeDoc(doc);
  return result;
}

ORCA_API lpObject_t
FS_ParseObjectFromXMLString(lua_State* L, lpcString_t xml_string)
{
  xmlDoc* doc = xmlReadMemory(xml_string, (int)strlen(xml_string),
                              NULL, NULL, XML_FLAGS);
  if (!doc) {
    Con_Printf("FS_ParseObjectFromXMLString: failed to parse XML string");
    return NULL;
  }
  xmlNodePtr root = xmlDocGetRootElement(doc);
  lpObject_t result = root ? FS_ConstructNode(L, root) : NULL;
  xmlFreeDoc(doc);
  return result;
}
