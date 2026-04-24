#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <source/core/core_properties.h>

// Forward declarations for the node-construction functions
static lpObject_t _FS_ConstructNode(xmlNodePtr element, bool_t send_start);
static lpObject_t FS_ConstructNode(xmlNodePtr element);

// External declaration of the property string parser defined in core/property/property_flow.c
// parse_property handles all common property types (bool, int, float, string, color, enum,
// struct) in pure C without a Lua state.  For kDataTypeObject it calls FS_LoadObjectFromXml.
extern int parse_property(const char* str,
                           struct PropertyType const* prop, void* valueptr);

// --- Helpers ----------------------------------------------------------------

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
_SetPropertyFromString(lpObject_t obj, lpcString_t name, lpcString_t value)
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
    Con_Error("Unknown property '%s' for class '%s'",
               name, OBJ_GetClassName(obj));
    return;
  }

  // Find or create the property slot
  lpProperty_t prop = NULL;
  if (FAILED(OBJ_FindLongProperty(obj, pdesc->FullIdentifier, &prop))) {
    Con_Error("Could not get property slot for '%s'", pdesc->Name);
    return;
  }

  // Resolve theme variables: "$accent" → FS_GetThemeValue("$accent")
  lpcString_t resolved = (value[0] == '$') ? FS_GetThemeValue(value) : NULL;
  if (!resolved) resolved = value;

  // Parse the string value into a stack-allocated buffer using pure-C parse_property.
  // Handles bool, int, float, string, color, enum, struct (via OBJ_ParseStruct),
  // and kDataTypeObject (via FS_LoadObjectFromXml).
  char tmpbuf[MAX_PROPERTY_STRING] = {0};
  if (!parse_property(resolved, pdesc, tmpbuf)) return;

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
_ConstructProperty(lpObject_t obj,
                   lpcPropertyType_t pdesc, xmlNodePtr element)
{
  // Array properties require Lua metatables for struct construction.
  // Skip with a diagnostic; callers can use the Lua API if needed.
  if (pdesc->IsArray) {
    Con_Error("array property '%s' not supported in C-only XML parser - skipped",
               pdesc->Name);
    return;
  }

  // Non-array case: check for a Value attribute first
  xmlChar* val = xmlGetProp(element, XMLSTR("Value"));
  if (val) {
    _SetPropertyFromString(obj, pdesc->Name, (lpcString_t)val);
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
_HandleAnimationPlayer(lpObject_t obj, xmlNodePtr element)
{
  OBJ_AddComponent(obj, ID_AnimationPlayer);
  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(obj, (lpcString_t)attr->name, (lpcString_t)val);
      xmlFree(val);
    }
  }
}

// Handle <AnimationCurve ...> child: regular child object of an AnimationClip.
static void
_HandleAnimationCurve(lpObject_t obj, xmlNodePtr element)
{
  lpcClassDesc_t cls = OBJ_FindClass("AnimationCurve");
  if (!cls) {
    Con_Error("AnimationCurve class not found");
    return;
  }
  lpObject_t curve = OBJ_Create(cls);

  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(curve, (lpcString_t)attr->name, (lpcString_t)val);
      xmlFree(val);
    }
  }

  // Process explicit-property child elements (e.g. <AnimationCurve.Keyframes>)
  xmlForEach(sub, element) {
    lpcPropertyType_t pdesc = OBJ_FindExplicitProperty(curve, (lpcString_t)sub->name);
    if (pdesc) {
      _ConstructProperty(curve, pdesc, sub);
    }
  }

  OBJ_AddChild(obj, curve, FALSE);
  OBJ_SendMessageW(curve, ID_Object_Start, 0, NULL);
}

// Handle <StateManagerController ...> child: attach-only component with an
// optional inline <StateManager> child element.
static void
_HandleStateManagerController(lpObject_t obj, xmlNodePtr element)
{
  OBJ_AddComponent(obj, ID_StateManagerController);

  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(obj, (lpcString_t)attr->name, (lpcString_t)val);
      xmlFree(val);
    }
  }

  // Look for an inline <StateManager> child element
  xmlFindAll(sub, element, XMLSTR("StateManager")) {
    lpObject_t sm = FS_ConstructNode(sub);
    if (sm) {
      lpProperty_t prop = NULL;
      if (SUCCEEDED(OBJ_FindShortProperty(obj, "StateManager", &prop))) {
        PROP_SetValue(prop, &sm);
      }
    }
    break; // only the first <StateManager> is used
  }
}

// Handle <LayerPrefabPlaceholder> and <ObjectPrefabPlaceholder>: load the
// linked XML file (PlaceholderTemplate attribute) in-place and apply the
// remaining attributes to the loaded object before dispatching Object.Start.
// No Lua state is required.
static lpObject_t
_HandlePrefabPlaceholder(xmlNodePtr element)
{
  xmlChar* tmpl = xmlGetProp(element, XMLSTR("PlaceholderTemplate"));
  if (!tmpl) {
    Con_Error("PrefabPlaceholder missing PlaceholderTemplate attribute");
    return NULL;
  }
  // Load the template XML doc manually so we can defer Object.Start on the
  // root until after placeholder attribute overrides have been applied.
  return FS_LoadObjectFromXml((char*)tmpl);
}

// Build an Object tree from an XML element node.
// send_start: when TRUE, dispatches Object.Start on the root after construction.
//             Set FALSE in _HandlePrefabPlaceholder to defer Start until after
//             placeholder attribute overrides have been applied.
static lpObject_t
_FS_ConstructNode(xmlNodePtr element, bool_t send_start)
{
  lpcString_t tag = (lpcString_t)element->name;

  // PrefabPlaceholder: handled separately; it manages Start dispatch itself
  if (!strcmp(tag, "LayerPrefabPlaceholder") ||
      !strcmp(tag, "ObjectPrefabPlaceholder") ||
      !strcmp(tag, "LibraryPlaceholder"))
  {
    return _HandlePrefabPlaceholder(element);
  }

  lpcClassDesc_t cls = OBJ_FindClass(tag);
  if (!cls) {
    Con_Error("Unknown element type '%s'", tag);
    return NULL;
  }

  lpObject_t obj = OBJ_Create(cls);

  // Parse all XML attributes as object properties
  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(obj, (lpcString_t)attr->name, (lpcString_t)val);
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
    if (send_start) OBJ_SendMessageW(obj, ID_Object_Start, 0, NULL);
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
      _ConstructProperty(obj, pdesc, sub);
      continue;
    }

    // Special attach-only components and legacy elements
    if (!strcmp(tag, "AnimationPlayer")) {
      _HandleAnimationPlayer(obj, sub);
    } else if (!strcmp(tag, "AnimationCurve")) {
      _HandleAnimationCurve(obj, sub);
    } else if (!strcmp(tag, "StateManagerController")) {
      _HandleStateManagerController(obj, sub);
    } else if (!strcmp(tag, "script")     ||
               !strcmp(tag, "EventListener") ||
               !strcmp(tag, "StyleSheet")    ||
               !strcmp(tag, "ValueTicker")) {
      // Lua-only elements: no-op in the C XML parser
    } else {
      // Regular child object: recurse and attach (children always get Start)
      lpObject_t child = _FS_ConstructNode(sub, TRUE);
      if (child) {
        OBJ_AddChild(obj, child, FALSE);
      }
    }
  }

  if (send_start) OBJ_SendMessageW(obj, ID_Object_Start, 0, NULL);
  return obj;
}

// Public wrapper: always sends Object.Start for the root.
static lpObject_t
FS_ConstructNode(xmlNodePtr element)
{
  return _FS_ConstructNode(element, TRUE);
}

lpObject_t
FS_LoadObjectFromXml(lpcString_t path)
{
  struct file* fp = FS_LoadFile(path);
  if (fp) {
    xmlDoc* doc = xmlReadMemory((lpcString_t)fp->data, (int)fp->size,
                                path, NULL, XML_FLAGS);
    FS_FreeFile(fp);
    if (!doc) {
      Con_Error("FS_LoadObjectFromXml: failed to parse '%s'", path);
      return NULL;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    lpObject_t result = root ? FS_ConstructNode(root) : NULL;
    xmlFreeDoc(doc);
    if (result) {
      OBJ_SetSourceFile(result, path);
//      OBJ_RegisterPrefab(result, path);
    }
    return result;
  }
  return NULL;
}

ORCA_API lpObject_t
FS_LoadObjectFromXML(lpcString_t path)
{
  return FS_LoadObjectFromXml(path);
}

ORCA_API lpObject_t
FS_LoadObjectFromXmlString(lpcString_t xml_string)
{
  xmlDoc* doc = xmlReadMemory(xml_string, (int)strlen(xml_string),
                              NULL, NULL, XML_FLAGS);
  if (!doc) {
    Con_Error("FS_LoadObjectFromXmlString: failed to parse XML string");
    return NULL;
  }
  xmlNodePtr root = xmlDocGetRootElement(doc);
  lpObject_t result = root ? FS_ConstructNode(root) : NULL;
  xmlFreeDoc(doc);
  return result;
}

ORCA_API lpObject_t
FS_LoadObjectFromXMLString(lpcString_t xml_string)
{
  return FS_LoadObjectFromXmlString(xml_string);
}
