#include <ctype.h>
#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <source/core/core_properties.h>
#include "fs_xml_inline.h"

// Forward declarations for the node-construction functions
static struct Object *_FS_ConstructNode(xmlNodePtr element, bool_t send_start);
struct Object *FS_ConstructNode(xmlNodePtr element);

// External declaration of the property string parser defined in core/property/property_flow.c
// parse_property handles all common property types (bool, int, float, string, color, enum,
// struct) in pure C without a Lua state.  For kDataTypeObject it calls FS_LoadObjectFromXml.
extern int parse_property(const char* str,
                           struct PropertyType const* prop, void* valueptr);

static lpcString_t
_SkipSpace(lpcString_t s)
{
  while (*s && isspace((unsigned char)*s)) s++;
  return s;
}

static void
_TrimView(lpcString_t in, lpcString_t *begin, lpcString_t *end)
{
  lpcString_t b = in;
  lpcString_t e = in + strlen(in);
  while (b < e && isspace((unsigned char)*b)) b++;
  while (e > b && isspace((unsigned char)*(e - 1))) e--;
  *begin = b;
  *end = e;
}

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

static bool_t
_MakeBindingExpr(lpcString_t input, fixedString_t out)
{
  lpcString_t begin = NULL;
  lpcString_t end = NULL;
  _TrimView(input, &begin, &end);
  if (begin == end) {
    return FALSE;
  }

  if ((size_t)(end - begin) > 8 && !strncmp(begin, "{Binding", 8) && *(end - 1) == '}') {
    lpcString_t path_begin = begin + 8;
    lpcString_t path_end = end - 1;
    _TrimView(path_begin, &path_begin, &path_end);
    if (path_begin >= path_end) return FALSE;

    fixedString_t path = {0};
    size_t path_len = (size_t)(path_end - path_begin);
    if (path_len >= sizeof(path)) return FALSE;
    memcpy(path, path_begin, path_len);
    path[path_len] = '\0';

    if (!strncmp(path, "Template/", 9)) {
      int n = snprintf(out, sizeof(fixedString_t), "{##Template/%s}", path + 9);
      return n > 0 && n < (int)sizeof(fixedString_t);
    }
    if (!strncmp(path, "##Template/", 11) ||
        !strncmp(path, "##Root/", 7) ||
        !strncmp(path, "DataContext/", 12) ||
        *path == '.' || *path == '#') {
      int n = snprintf(out, sizeof(fixedString_t), "{%s}", path);
      return n > 0 && n < (int)sizeof(fixedString_t);
    }

    {
      int n = snprintf(out, sizeof(fixedString_t), "{##Template/%s}", path);
      return n > 0 && n < (int)sizeof(fixedString_t);
    }
  }

  if (*begin == '{' && *(end - 1) == '}') {
    size_t n = (size_t)(end - begin);
    if (n >= sizeof(fixedString_t)) return FALSE;
    memcpy(out, begin, n);
    out[n] = '\0';
    return TRUE;
  }

  lpcString_t path_begin = begin;
  lpcString_t path_end = end;

  fixedString_t path = {0};
  size_t path_len = (size_t)(path_end - path_begin);
  if (path_len >= sizeof(path)) return FALSE;
  memcpy(path, path_begin, path_len);
  path[path_len] = '\0';

  if (!strncmp(path, "Template/", 9)) {
    int n = snprintf(out, sizeof(fixedString_t), "{##Template/%s}", path + 9);
    return n > 0 && n < (int)sizeof(fixedString_t);
  }
  if (!strncmp(path, "##Template/", 11) ||
      !strncmp(path, "##Root/", 7) ||
      !strncmp(path, "DataContext/", 12) ||
      *path == '.' || *path == '#') {
    int n = snprintf(out, sizeof(fixedString_t), "{%s}", path);
    return n > 0 && n < (int)sizeof(fixedString_t);
  }

  {
    int n = snprintf(out, sizeof(fixedString_t), "{##Template/%s}", path);
    return n > 0 && n < (int)sizeof(fixedString_t);
  }
}

static bool_t
_AttachPropertyProgramFromExpression(struct Object *obj,
                                     lpcString_t prop_name,
                                     lpcString_t expression,
                                     xmlNodePtr attr_source,
                                     xmlNodePtr fallback_attr_source)
{
  enum PropertyAttribute attribute = kPropertyAttributeWholeProperty;
  enum BindingMode mode = kBindingModeExpression;
  bool_t enabled = TRUE;

  xmlChar* attr_xml = attr_source ? xmlGetProp(attr_source, XMLSTR("Attribute")) : NULL;
  xmlChar* mode_xml = attr_source ? xmlGetProp(attr_source, XMLSTR("Mode")) : NULL;
  xmlChar* en_xml   = attr_source ? xmlGetProp(attr_source, XMLSTR("Enabled")) : NULL;

  if (!attr_xml && fallback_attr_source) attr_xml = xmlGetProp(fallback_attr_source, XMLSTR("Attribute"));
  if (!mode_xml && fallback_attr_source) mode_xml = xmlGetProp(fallback_attr_source, XMLSTR("Mode"));
  if (!en_xml && fallback_attr_source) en_xml = xmlGetProp(fallback_attr_source, XMLSTR("Enabled"));

  if (attr_xml) {
    attribute = _ParseAttributeStr((lpcString_t)attr_xml);
    xmlFree(attr_xml);
  }
  if (mode_xml) {
    mode = _ParseBindingModeStr((lpcString_t)mode_xml);
    xmlFree(mode_xml);
  }
  if (en_xml) {
    enabled = strcmp((lpcString_t)en_xml, "false") != 0;
    xmlFree(en_xml);
  }

  return OBJ_AttachPropertyProgram(obj, prop_name, expression,
                                   attribute, mode, enabled);
}

// Parse a single XML attribute value and set it on obj.
// Handles Name/id, class, and all registered properties.
static void
_SetPropertyFromString(struct Object *obj, lpcString_t name, lpcString_t value)
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
  struct PropertyType const *pdesc = OBJ_FindImplicitProperty(obj, name);
  if (!pdesc) pdesc = OBJ_FindExplicitProperty(obj, name);
  if (!pdesc) {
    Con_Error("Unknown property '%s' for class '%s'",
               name, OBJ_GetClassName(obj));
    return;
  }

  // Find or create the property slot
  struct Property *prop = NULL;
  if (FAILED(OBJ_FindLongProperty(obj, pdesc->FullIdentifier, &prop))) {
    Con_Error("Could not get property slot for '%s'", pdesc->Name);
    return;
  }

  // Resolve theme variables: "$accent" → FS_GetThemeValue("$accent")
  lpcString_t resolved = (value[0] == '$') ? FS_GetThemeValue(value) : NULL;
  if (!resolved) resolved = value;

  // Syntax sugar: {Binding Card.Title} => {##Template/Card.Title}
  // This attaches a property program rather than treating it as a literal.
  if (pdesc->DataType != kDataTypeEvent) {
    fixedString_t binding_expr = {0};
    if (_MakeBindingExpr(resolved, binding_expr) && !strncmp(_SkipSpace(resolved), "{Binding", 8)) {
      _AttachPropertyProgramFromExpression(obj, pdesc->Name, binding_expr, NULL, NULL);
      return;
    }
  }

  // XML object references: "@../Popup" resolves against the current object.
  // This keeps trigger/action wiring declarative without requiring file loads.
  if (pdesc->DataType == kDataTypeObject && resolved[0] == '@') {
    struct Object *ref = OBJ_FindByPath(obj, resolved + 1);
    if (!ref) {
      Con_Error("Could not resolve object reference '%s' for property '%s'",
                 resolved + 1, pdesc->Name);
      return;
    }
    PROP_SetValue(prop, &ref);
    return;
  }

  // Inline object expressions can be assigned directly to object properties.
  // The fragment is parsed as a self-contained XML document and constructed
  // the same way as a file-backed object.
  lpcString_t inline_expr = resolved;
  while (*inline_expr && isspace((unsigned char)*inline_expr)) inline_expr++;
  if (pdesc->DataType == kDataTypeEvent && *inline_expr == '{') {
    if (_LoadEventTriggerFromXmlFragment(obj, pdesc, inline_expr)) {
      return;
    }
  }
  if (pdesc->DataType == kDataTypeStruct && *inline_expr == '{') {
    if (_LoadStructFromXmlFragment(prop, pdesc, inline_expr, pdesc->Name)) {
      return;
    }
  }
  if (pdesc->DataType == kDataTypeObject &&
      (*inline_expr == '<' || *inline_expr == '{')) {
    struct Object *inline_obj = _LoadObjectFromXmlFragment(inline_expr, pdesc->Name, 0);
    if (!inline_obj) {
      return;
    }
    PROP_SetValue(prop, &inline_obj);
    return;
  }

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

// Build an object-property child element (e.g. <Grid.Columns>).
static void
_ConstructProperty(struct Object *obj,
                   struct PropertyType const *pdesc, xmlNodePtr element)
{
  // Array properties are built from child elements. Object arrays instantiate
  // child objects; struct arrays populate struct blobs from attributes.
  if (pdesc->IsArray) {
    if (pdesc->DataType != kDataTypeObject && pdesc->DataType != kDataTypeStruct) {
      Con_Error("array property '%s' not supported in C-only XML parser - skipped",
                pdesc->Name);
      return;
    }

    struct Property *prop = NULL;
    if (FAILED(OBJ_FindLongProperty(obj, pdesc->FullIdentifier, &prop))) {
      Con_Error("Could not get array property slot for '%s'", pdesc->Name);
      return;
    }
    int numitems = 0;
    xmlForEach(sub, element) {
      xmlChar* dis = xmlGetProp(sub, XMLSTR("IsDisabled"));
      bool_t is_disabled = dis && !xmlStrcmp(dis, XMLSTR("true"));
      if (dis) xmlFree(dis);
      if (!is_disabled && sub->type == XML_ELEMENT_NODE) {
        numitems++;
      }
    }

    void *items = numitems ? calloc((size_t)numitems, (size_t)pdesc->DataSize) : NULL;
    struct StructDesc const *sdesc = NULL;
    if (pdesc->DataType == kDataTypeStruct) {
      sdesc = OBJ_FindStructDesc(pdesc->TypeString);
      if (!sdesc) {
        Con_Error("Could not resolve struct descriptor '%s' for array property '%s'",
                  pdesc->TypeString, pdesc->Name);
        free(items);
        return;
      }
    }
    if (numitems > 0 && !items) {
      Con_Error("Could not allocate array storage for property '%s'", pdesc->Name);
      return;
    }

    int index = 0;
    xmlForEach(sub, element) {
      if (sub->type != XML_ELEMENT_NODE) {
        continue;
      }
      xmlChar* dis = xmlGetProp(sub, XMLSTR("IsDisabled"));
      bool_t is_disabled = dis && !xmlStrcmp(dis, XMLSTR("true"));
      if (dis) xmlFree(dis);
      if (is_disabled) {
        continue;
      }

      if (pdesc->DataType == kDataTypeObject) {
        struct Object *child = FS_ConstructNode(sub);
        if (!child) {
          continue;
        }
        if (items) {
          ((struct Object **)items)[index++] = child;
        }
      } else {
        void *dst = items ? (char*)items + (size_t)index * pdesc->DataSize : NULL;
        void *tmp = sdesc ? calloc(1, (size_t)sdesc->StructSize) : NULL;
        if (!tmp) {
          Con_Error("Could not allocate struct storage for '%s' while parsing array property '%s'",
                    sdesc->StructName, pdesc->Name);
          continue;
        }
        bool_t ok = TRUE;
        FOR_EACH_LIST(xmlAttr, attr, sub->properties) {
          xmlChar* val = xmlGetProp(sub, attr->name);
          if (!val) {
            continue;
          }
          struct PropertyType const *field = _FindStructField(sdesc, (lpcString_t)attr->name);
          if (!field) {
            Con_Error("Unknown field '%s' for struct '%s' in array property '%s'",
                      attr->name, sdesc->StructName, pdesc->Name);
            xmlFree(val);
            continue;
          }
          if (!_SetStructFieldFromString(field, (char*)tmp + field->Offset, (lpcString_t)val)) {
            ok = FALSE;
          }
          xmlFree(val);
        }
        if (ok && tmp && dst) {
          memcpy(dst, tmp, (size_t)sdesc->StructSize);
          index++;
        }
        if (!ok && tmp) {
          _ClearStructValue(sdesc, tmp);
        }
        free(tmp);
      }
    }

    struct {
      void *items;
      int count;
    } array_value = { .items = items, .count = index };
    PROP_SetValue(prop, &array_value);
    return;
  }

  // Non-array case: check for a Value attribute first
  xmlChar* val = xmlGetProp(element, XMLSTR("Value"));
  if (val) {
    _SetPropertyFromString(obj, pdesc->Name, (lpcString_t)val);
    xmlFree(val);
    return;
  }

  // Allow implicit text-content expressions (backward compatible):
  //   <Some.Property>{../Card.Title}</Some.Property>
  //   <Some.Property>IF(STEP(...), "auto", "manual")</Some.Property>
  xmlChar* text = xmlNodeGetContent(element);
  lpcString_t expr_text = text ? _SkipSpace((lpcString_t)text) : NULL;
  if (expr_text && *expr_text) {
    fixedString_t expr = {0};
    if (_MakeBindingExpr(expr_text, expr) && strstr(expr_text, "{Binding") == expr_text) {
      _AttachPropertyProgramFromExpression(obj, pdesc->Name, expr, NULL, NULL);
    } else if (expr_text[0] == '{' || !strncmp(expr_text, "IF(", 3) || !strncmp(expr_text, "STEP(", 5)) {
      _AttachPropertyProgramFromExpression(obj, pdesc->Name, expr_text, NULL, NULL);
    } else {
      Con_Error("Property '%s' has unexpected text content (not an expression); use Value=\"...\" instead",
                pdesc->Name);
    }
  }
  if (text) xmlFree(text);
}

// Handle an attach-only component element generically: attach the component to
// obj, apply all XML attributes as properties on the parent object, and process
// any child elements that correspond to explicit properties on the parent or to
// inline child objects whose tag name matches a property of the parent.
static void
_HandleAttachOnlyComponent(struct Object *obj, xmlNodePtr element,
                           struct ClassDesc const *cls)
{
  OBJ_AddComponent(obj, cls->ClassID);

  // Apply XML attributes as properties on the parent object.
  FOR_EACH_LIST(xmlAttr, attr, element->properties) {
    xmlChar* val = xmlGetProp(element, attr->name);
    if (val) {
      _SetPropertyFromString(obj, (lpcString_t)attr->name, (lpcString_t)val);
      xmlFree(val);
    }
  }

  // Process child elements: explicit properties of the parent, or inline child
  // objects assigned to a parent property whose name matches the element tag.
  xmlForEach(sub, element) {
    lpcString_t subtag = (lpcString_t)sub->name;

    struct ClassDesc const *subcls = OBJ_FindClass(subtag);
    if (subcls && subcls->IsAttachOnly) {
      _HandleAttachOnlyComponent(obj, sub, subcls);
      continue;
    }

    struct PropertyType const *pdesc = OBJ_FindExplicitProperty(obj, subtag);
    if (pdesc) {
      _ConstructProperty(obj, pdesc, sub);
      continue;
    }

    if (!strcmp(subtag, "Binding") ||
        !strcmp(subtag, "BindingExpression") ||
        !strcmp(subtag, "ExpressionBinding")) {
      continue;
    }

    // Try to construct as a child object and assign to the matching property.
    struct Object *child = FS_ConstructNode(sub);
    if (child) {
      struct Property *prop = NULL;
      if (SUCCEEDED(OBJ_FindShortProperty(obj, subtag, &prop))) {
        PROP_SetValue(prop, &child);
      } else {
        OBJ_AddChild(obj, child, FALSE);
      }
    }
  }
}

// Handle <LayerPrefabPlaceholder> and <ObjectPrefabPlaceholder>: load the
// linked XML file (PlaceholderTemplate attribute) in-place and apply the
// remaining attributes to the loaded object before dispatching Object.Start.
// No Lua state is required.
static struct Object *
_HandlePrefabPlaceholder(xmlNodePtr element)
{
  xmlChar* tmpl = xmlGetProp(element, XMLSTR("PlaceholderTemplate"));
  if (!tmpl) {
    Con_Error("PrefabPlaceholder missing PlaceholderTemplate attribute");
    return NULL;
  }
  // Load the template XML doc manually so we can defer Object.Start on the
  // root until after placeholder attribute overrides have been applied.
  struct Object *obj = FS_LoadObject((char*)tmpl);
  xmlFree(tmpl);
  if (obj) {
    OBJ_SetFlags(obj, OBJ_GetFlags(obj) | OF_TEMPLATE);
  }
  return obj;
}

// Build an Object tree from an XML element node.
// send_start: when TRUE, dispatches Object.Start on the root after construction.
//             Set FALSE in _HandlePrefabPlaceholder to defer Start until after
//             placeholder attribute overrides have been applied.
static struct Object *
_FS_ConstructNode(xmlNodePtr element, bool_t send_start)
{
  lpcString_t tag = (lpcString_t)element->name;
  struct Object *obj = NULL;
  bool_t prefab = FALSE;
  
  // PrefabPlaceholder: handled separately; it manages Start dispatch itself
  if (!strcmp(tag, "LayerPrefabPlaceholder") ||
      !strcmp(tag, "ObjectPrefabPlaceholder") ||
      !strcmp(tag, "LibraryPlaceholder"))
  {
    obj = _HandlePrefabPlaceholder(element);
    prefab = TRUE;
  }
  else
  {
    struct ClassDesc const *cls = OBJ_FindClass(tag);
    if (!cls) {
      Con_Error("Unknown element type '%s'", tag);
      return NULL;
    }
    obj = OBJ_Create(cls->ClassID);
  }
  
  if (!obj) {
    Con_Error("Can not allocate object");
    return NULL;
  }

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
    struct PropertyType const *pdesc = OBJ_FindExplicitProperty(obj, tag);
    if (pdesc) {
      _ConstructProperty(obj, pdesc, sub);
      continue;
    }

    struct ClassDesc const *subcls = OBJ_FindClass(tag);
    if (subcls && subcls->IsAttachOnly) {
      _HandleAttachOnlyComponent(obj, sub, subcls);
      continue;
    }

    // Non-constructible declarative elements: handled elsewhere or by Lua.
    if (!strcmp(tag, "script")        ||
        !strcmp(tag, "EventListener") ||
        !strcmp(tag, "StyleSheet")    ||
        !strcmp(tag, "ValueTicker")   ||
        !strcmp(tag, "Binding")       ||
        !strcmp(tag, "BindingExpression") ||
        !strcmp(tag, "ExpressionBinding")) {
      continue;
    }

    if (!prefab) {
      // Regular child object: recurse and attach (children always get Start)
      struct Object *child = _FS_ConstructNode(sub, TRUE);
      if (child) {
        OBJ_AddChild(obj, child, FALSE);
      }
    } else {
      Con_Error("Can't parse node %s", tag);
    }
  }

  // Second pass: handle explicit top-level BindingExpression, legacy
  // ExpressionBinding, and Binding elements.
  xmlForEach(sub, element) {
    lpcString_t tag = (lpcString_t)sub->name;
    if (!strcmp(tag, "BindingExpression") ||
        !strcmp(tag, "ExpressionBinding") ||
        !strcmp(tag, "Binding")) {
      xmlChar* target_attr = xmlGetProp(sub, XMLSTR("Target"));
      if (!target_attr) {
        Con_Error("<%s> element is missing required Target attribute", tag);
        continue;
      }
      xmlChar* expr_text = xmlNodeGetContent(sub);
      lpcString_t expr = expr_text ? _SkipSpace((lpcString_t)expr_text) : NULL;
      if (!expr || !*expr) {
        Con_Error("<%s Target=\"%s\"> has empty content", tag, target_attr);
        xmlFree(target_attr);
        if (expr_text) xmlFree(expr_text);
        continue;
      }

      fixedString_t normalized_expr = {0};
      if (!strcmp(tag, "Binding")) {
        if (!_MakeBindingExpr(expr, normalized_expr)) {
          Con_Error("<%s Target=\"%s\"> has malformed binding expression", tag, target_attr);
          xmlFree(target_attr);
          xmlFree(expr_text);
          continue;
        }
        _AttachPropertyProgramFromExpression(obj, (lpcString_t)target_attr, normalized_expr, NULL, NULL);
      } else {
        _AttachPropertyProgramFromExpression(obj, (lpcString_t)target_attr, expr, NULL, NULL);
      }
      xmlFree(target_attr);
      if (expr_text) xmlFree(expr_text);
    }
  }

  if (send_start) OBJ_SendMessageW(obj, ID_Object_Start, 0, NULL);
  return obj;
}

// Public wrapper: always sends Object.Start for the root.
struct Object *
FS_ConstructNode(xmlNodePtr element)
{
  return _FS_ConstructNode(element, TRUE);
}

struct Object *
FS_LoadObjectFromXml(lpcString_t path)
{
  if (!path) return NULL;
  struct file* fp = FS_LoadFile(path);
  if (fp) {
    char *normalized = _NormalizeXmlShorthand((lpcString_t)fp->data);
    const char *xml_text = normalized ? normalized : (lpcString_t)fp->data;
    int xml_len = normalized ? (int)strlen(xml_text) : (int)fp->size;
    xmlDoc* doc = xmlReadMemory(xml_text, xml_len,
                                path, NULL, XML_FLAGS);
    FS_FreeFile(fp);
    if (!doc) {
      free(normalized);
      Con_Error("Failed to parse '%s'", path);
      return NULL;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    struct Object *result = root ? FS_ConstructNode(root) : NULL;
    xmlFreeDoc(doc);
    free(normalized);
    if (result) {
      OBJ_SetSourceFile(result, path);
//      OBJ_RegisterPrefab(result, path);
    }
    return result;
  } else {
    Con_Error("Failed to load '%s'", path);
    return NULL;
  }
  return NULL;
}

ORCA_API struct Object *
FS_LoadObjectFromXML(lpcString_t path)
{
  return FS_LoadObjectFromXml(path);
}

ORCA_API struct Object *
FS_LoadObjectFromXmlString(lpcString_t xml_string)
{
  char *normalized = _NormalizeXmlShorthand(xml_string);
  const char *xml_text = normalized ? normalized : xml_string;
  xmlDoc* doc = xmlReadMemory(xml_text, normalized ? (int)strlen(xml_text) : (int)strlen(xml_string),
                              NULL, NULL, XML_FLAGS);
  if (!doc) {
    free(normalized);
    Con_Error("FS_LoadObjectFromXmlString: failed to parse XML string");
    return NULL;
  }
  xmlNodePtr root = xmlDocGetRootElement(doc);
  struct Object *result = root ? FS_ConstructNode(root) : NULL;
  xmlFreeDoc(doc);
  free(normalized);
  return result;
}

ORCA_API struct Object *
FS_LoadObjectFromXMLString(lpcString_t xml_string)
{
  return FS_LoadObjectFromXmlString(xml_string);
}
