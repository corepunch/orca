#include "xml_local.h"

#include <include/renderer.h>

#define InitProperty(pobj, szKey, type, value)                           \
PROP_SetValue(PROP_Create(L, pobj, szKey, type, NULL), value)

#define MAX_COMPONETNS MAX_PROPERTY_STRING / sizeof(float)

static lpcString_t _edges[4] = { "Top", "Right", "Bottom", "Left" };

shortStr_t tags[MAX_TAGS] = {0};

extern bool_t FS_FileExists(lpcString_t);
extern lpObject_t OBJ_LoadDocument(lua_State *L, xmlDocPtr);
extern lpObject_t OBJ_FindKnownPrefab(lpcString_t, lpcString_t*);

struct token*
Token_Create(lpcString_t code)
{
  struct token* _compile(lpcString_t code, lpcString_t filename);
  return _compile(code, "binding");
}


static void
PROP_ParseObjectValue(lua_State *L,
                      lpProperty_t prop,
                      lpcString_t path)
{
  path_t tmp={0};
  strncpy(tmp, path, sizeof(tmp)-1);
  strcat(tmp, "/");
  lpObject_t known = OBJ_FindKnownPrefab(tmp, NULL);
  if (known) {
    PROP_SetValue(prop, &known);
    return;
  }
  xmlWith(xmlDoc, doc, FS_LoadXML(path), xmlFreeDoc) {
    lpObject_t object = OBJ_LoadDocument(L, doc);
    if (!object) {
      Con_Error("Can't load object %s", path);
      continue;
    }
    PROP_SetValue(prop, &object);
  }
}

int
PROP_Parse2(lua_State* L,
            lpObject_t pobj,
            lpcString_t szKey,
            lpcString_t szValue)
{
  lpProperty_t hProperty = NULL;
  if (!strcmp(szKey, "id") || !strcmp(szKey, "Name")) {
    OBJ_SetName(pobj, szValue);
    return 1;
  }
  if (!strcmp(szKey, "class")) {
    OBJ_ParseClassAttribute(pobj, szValue);
    return 1;
  }
  if (!strcmp(szKey, "IsDisabled")) {
    return 0;
  }
  if (!strcmp(szKey, "StateManager") || !strcmp(szKey, "Node.StateManager")) {
    SM_ReadStateManager(L, pobj, FS_LoadXML(szValue), PROP_Parse2);
    return 1;
  }
  if (!strcmp(szKey, "Style") || !strcmp(szKey, "Node.Style")) {
    if (strrchr(szValue, '/')) {
      OBJ_ParseClassAttribute(pobj, strrchr(szValue, '/') + 1);
    } else {
      OBJ_ParseClassAttribute(pobj, szValue);
    }
    return 1;
  }
  //  if (!strncmp(szKey, "on", 2) && szKey[2] >= 'A' && szKey[2] <= 'Z') {
  //    InitProperty(pobj, &szKey[2], kDataTypeEvent, MAX_PROPERTY_STRING,
  // szValue);     lua_pushboolean(L, TRUE);     return 1;
  //  }
  if (FAILED(OBJ_FindShortProperty(pobj, szKey, &hProperty))
      /* && FAILED(OBJ_FindLongProperty(pobj, szKey, &hProperty))*/)
  {
#ifdef SUPPORT_DYNAMIC_PROPERTIES
    if (*szValue == '{') {
      hProperty = PROP_Create(L, pobj, szKey, kDataTypeNone, NULL);
      PROP_AttachProgram(hProperty, 0, Token_Create(szValue), szValue);
      return 0;
    }
    LPSTR szEnd;
    float pNumbers[MAX_COMPONETNS] = { strtof(szValue, &szEnd) };
    uint32_t i = 1;
    if (szEnd != szValue) {
      eDataType_t datatype = -1;
      switch (*szEnd) {
        case 'd':
          InitProperty(pobj, szKey, kDataTypeInt, &(int){ *pNumbers });
          break;
        case 'b':
          InitProperty(pobj, szKey, kDataTypeBool, &(bool_t){ *pNumbers });
          break;
        case '\t':
        case ' ':
          for (; i < MAX_COMPONETNS && isspace(*szEnd); pNumbers[i++] = strtof(szEnd, &szEnd));
          switch (i) {
            case 1: datatype = kDataTypeFloat; break;
            case 2: datatype = kDataTypeVector2D; break;
            case 3: datatype = kDataTypeVector3D; break;
            case 4: datatype = kDataTypeVector4D; break;
//            case 9: datatype = kDataTypeMatrix2D; break;
            case 16: datatype = kDataTypeMatrix3D; break;
            case 5: datatype = kDataTypeTransform2D; break;
            case 9: datatype = kDataTypeTransform3D; break;
            default:
              assert(!"Incorrect amount of float components");
              return 0;
          }
          PROP_SetValue(PROP_Create(L, pobj, szKey, datatype, NULL), pNumbers);
          break;
        case '\0':
        case 'f':
          InitProperty(pobj, szKey, kDataTypeFloat, pNumbers);
          break;
        default:
          break;
      }
    } else if (!strcmp(szValue, "true")) {
      InitProperty(pobj, szKey, kDataTypeBool, &(bool_t){ TRUE });
    } else if (!strcmp(szValue, "false")) {
      InitProperty(pobj, szKey, kDataTypeBool, &(bool_t){ FALSE });
    } else if (*szValue == '#') {
      struct color color = COLOR_Parse(szValue);
      InitProperty(pobj, szKey, kDataTypeColor, &color);
    } else if (*szValue == '@' || *szValue == '\'') {
      InitProperty(pobj, szKey, kDataTypeFixed, &szValue[1]);
    } else if (!strchr(szValue, '/') || !strcmp(szKey, "Texts2DPath")) {
      InitProperty(pobj, szKey, kDataTypeFixed, szValue);
    } else {
      if (strstr(szValue, "/Fonts/")) {
        hProperty = PROP_Create(L, pobj, szKey, kDataTypeObject, "TrueTypeFont");
      } else if (strstr(szValue, "/Images/") || strstr(szValue, "/Textures/")) {
        hProperty = PROP_Create(L, pobj, szKey, kDataTypeObject, "Texture");
      } else if (strstr(szValue, "/Mesh Data/")) {
        hProperty = PROP_Create(L, pobj, szKey, kDataTypeObject, "Mesh");
      } else if (strstr(szValue, "/Render Passes/")) {
        hProperty = PROP_Create(L, pobj, szKey, kDataTypeObject, "RenderPass");
      } else if (strstr(szValue, "/Materials/")) {
        hProperty = PROP_Create(L, pobj, szKey, kDataTypeObject, "Material");
      } else if (strstr(szValue, "/Styles/")) {
        hProperty = PROP_Create(L, pobj, szKey, kDataTypeObject, "Style");
      } else if (strstr(szValue, "/Data Sources/")) {
        hProperty = PROP_Create(L, pobj, szKey, kDataTypeObject, "DataObject");
      } else if (strstr(szValue, "/Prefabs/")) {
        hProperty = PROP_Create(L, pobj, szKey, kDataTypeObject, "Object");
        return 0;
      } else if (strstr(szValue, "/Brushes/")) {
        struct color color = {1,1,0,1};
        InitProperty(pobj, szKey, kDataTypeColor, &color);
        Con_Warning("Using placeholder for %s for property %s", szValue, szKey);
        return 0;
      } else {
        Con_Warning("Can't parse %s for property %s", szValue, szKey);
        return 0;
        //        assert(FALSE);
      }
      if (PROP_GetType(hProperty) == kDataTypeObject) {
        PROP_ParseObjectValue(L, hProperty, szValue);
        return 0;
      } else {
        Con_Error("Unsupported type %d to load resource %s in property %s",
                PROP_GetType(hProperty), szValue, szKey);
//        uint32_t type = GetPropertyHandleType(PROP_GetUserData(hProperty));
//        handle_t hHandle = R_LoadResource(type, szValue);
//        PROP_SetValue(hProperty, &hHandle);
        return 0;
      }
    }
#else
    Con_Error("No definition found for attached property `%s` in %s(%s)\n", szKey, OBJ_GetName(pobj), OBJ_GetClassName(pobj));
#endif
    return 0;
  } else if (*szValue == '{') {
    PROP_AttachProgram(hProperty, 0, Token_Create(szValue), szValue);
    return 0;
  } else if (PROP_GetType(hProperty) == kDataTypeObject) {
    PROP_ParseObjectValue(L, hProperty, szValue);
    return 0;
  } else {
    PROP_Parse(hProperty, szValue);
  }
  
  //  if (*szValue == 0) {
  //    return 0;
  //  } else if (*szValue == '{') {
  //    struct token * Token_Create(lpcString_t );
  //    struct token * token = Token_Create(szValue);
  //    if (token) {
  //      assert(hProperty);
  //      PROP_AttachProgram(hProperty, 0, token);
  //      lua_pushboolean(L, TRUE);
  //    } else {
  //      lua_pushboolean(L, FALSE);
  //    }
  //    return 1;
  //  }
  
  //
  //  switch (PROP_GetType(property)) {
  //  case kDataTypeObject:
  //    break;
  //  case kDataTypeEvent:
  //    if (!strstr(szValue, "Changed")) {
  //      lua_pushvalue(L, 5);
  //      lua_pushvalue(L, 1);
  //      lua_pushstring(L, PROP_GetName(property));
  //      lua_pushstring(L, szValue);
  //      lua_pushvalue(L, 6);
  //      if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
  //        return luaL_error(L, lua_tostring(L, -1));
  //      }
  //    } else {
  //      PROP_Parse(property, szValue);
  //    }
  //    break;
  //  default:
  //    PROP_Parse(property, szValue);
  //    break;
  //  }
  return 1;
}


struct propshort {
  lpcString_t name;
  eDataType_t type;
  void const* userdata;
} props[] = {
  { "Int", kDataTypeInt },
  { "Int32", kDataTypeInt },
  { "Enum", kDataTypeInt },
  { "Float", kDataTypeFloat },
  { "Boolean", kDataTypeBool },
  { "Bool", kDataTypeBool },
  { "Color", kDataTypeColor },
  { "Vector2D", kDataTypeVector2D },
  { "Vector3D", kDataTypeVector3D },
  { "Vector4D", kDataTypeVector4D },
  { "Matrix2D", kDataTypeMatrix3D },
  { "Matrix3D", kDataTypeMatrix3D },
  { "String", kDataTypeFixed },
  { "Image", kDataTypeObject, "Texture" },
  { "Texture", kDataTypeObject, "Texture" },
  { "ProjectItemReference", kDataTypeObject, "Texture" },
};

lpProperty_t
PROP_CreateFromString(lua_State* L,
                      lpObject_t object,
                      lpcString_t name,
                      lpcString_t typestring)
{
  struct propshort def = { .type = kDataTypeNone };
  FOR_LOOP(i, sizeof(props)/sizeof(*props)) {
    if (!strcmp(typestring, props[i].name)) {
      def = props[i];
      break;
    }
  }
  if (!def.name) {
    Con_Error("Unknown property type %s for property %s", typestring, name);
  }
  return PROP_Create(L, object, name, def.type, def.userdata);
}

static uint32_t
tag_index(shortStr_t* tags, lpcString_t szName)
{
  uint32_t i = 1;
  for (; i < MAX_TAGS; i++) {
    if (!*tags[i])
      break;
    if (!strcmp(tags[i], szName))
      return i;
  }
  if (i == MAX_TAGS) {
    Con_Error("%s: can't register tag, all %d slots are used", szName, MAX_TAGS);
    return 0;
  }
  strncpy(tags[i], szName, sizeof(*tags));
  return i;
}

static objectTags_t
parse_tags(shortStr_t* _tags, lpcString_t value)
{
  objectTags_t tags = 0;
  for (LPSTR tok = strtok((LPSTR)value, ","); tok; tok = strtok(NULL, ",")) {
    SkipSpace(tok);
    tags |= 1 << tag_index(_tags, tok);
  }
  return tags;
}

static void
_ParseEdges(lpObject_t hobj,
            const struct PropertyDesc *pdesc,
            lpProperty_t p,
            lpcString_t string,
            void *dest)
{
  lpcString_t tmp[(sizeof(_edges) / sizeof(*_edges)) + 1] = { string };
  FOR_LOOP(i, sizeof(_edges) / sizeof(*_edges)) {
    shortStr_t buf = {0};
    snprintf(buf, sizeof(buf), pdesc->TypeString, _edges[i]);
    lpcPropertyDesc_t d = pdesc;
    for (; d->id && strcmp(d->id->Name, buf); d++);
    lpcString_t s = PDESC_Parse(hobj, d, p, tmp[i], dest + (d->Offset - pdesc->Offset));
    SkipSpace(s);
    tmp[i + 1] = *s ? s : tmp[(i + 1) % MIN(2, i + 1)];
  }
}

ORCA_API const char*
FS_GetBaseName(const char* path);

lpcString_t
PDESC_Parse(lpObject_t hobj,
            lpcPropertyDesc_t pdesc,
            lpProperty_t property,
            lpcString_t string,
            void* dest)
{
  //  lpcString_t corners[4] = { "TopLeft", "TopRight", "BottomRight", "BottomLeft" };
  float* v = dest;
  switch (pdesc->Flags&T_TYPEMASK) {
    case kDataTypeNone:
      break;
    case kDataTypeBool:
      ((int*)v)[0] = strcmp(string, "true") == 0;
      break;
    case kDataTypeEnum:
      if (isdigit(*string)) {
        *(uint32_t*)v = strtod(string, (LPSTR*)&string);
      } else {
        uint32_t index = strlistidx(string, pdesc->TypeString, &string);
        if (index == -1) {
          Con_Error("Could not parse '%s' value of property '%s'", string, pdesc->id->Name);
        } else {
          *(uint32_t*)v = index;
        }
      }
      break;
    case kDataTypeInt:
      ((int*)v)[0] = strtod(string, (LPSTR*)&string);
      break;
    case kDataTypeVector4D:
    case kDataTypeVector3D:
    case kDataTypeVector2D:
    case kDataTypeTransform2D:
    case kDataTypeTransform3D:
    case kDataTypeMatrix3D:
    case kDataTypeFloat:
      FOR_LOOP(i, PROP_GetSize(property)/sizeof(float)) {
        v[i] = *string ? strtof(string, (LPSTR*)&string) : v[0];
        if (!strncmp(string, "px", 2)) {
          string += 2;
        }
      }
      break;
    case kDataTypeFixed:
      if (strlen(string) >= MAX_PROPERTY_STRING) {
        Con_Error("String \"%s\" is longer than %d characters\n",string,MAX_PROPERTY_STRING);
      }
      strncpy(dest, string, MAX_PROPERTY_STRING);
      break;
    case kDataTypeLongString:
      *((void**)dest) = strdup(string);
      break;
    case kDataTypeEvent:
      strncpy(dest, string, MAX_PROPERTY_STRING);
      break;
    case kDataTypeColor:
      if (*string == '#') {
        struct color color = COLOR_Parse(string);
        memcpy(v, &color, sizeof(struct color));
      } else {
        sscanf(string, "%f%f%f%f", v + 0, v + 1, v + 2, v + 3);
      }
      break;
    case kDataTypeEdges:
      _ParseEdges(hobj, pdesc, property, string, dest);
      break;
    case kDataTypeObjectTags:
      *((objectTags_t*)dest) = parse_tags(tags, string);
      break;
    case kDataTypeObject:
      if (FS_FileExists(string)) {
        xmlWith(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFreeDoc) {
          reqArg_t args[16]={0};
          xmlWith(char, str, strdup(string), free) {
            lpcString_t path=FS_ParseArgs(str, args, sizeof(args)/sizeof(*args));
            xmlNodePtr root = xmlNewNode(NULL, XMLSTR("Image"));
            xmlSetProp(root, XMLSTR("Name"), XMLSTR(FS_GetBaseName(path)));
            xmlSetProp(root, XMLSTR("Source"), XMLSTR(path));
            for (reqArg_t *it = args; it-args<sizeof(args)/sizeof(*args);it++) {
              if (*it->name) {
                *it->name = toupper(*it->name);
                xmlSetProp(root, XMLSTR(it->name), XMLSTR(it->value));
              }
            }
            xmlDocSetRootElement(doc, root);
            doc->URL = xmlStrdup(XMLSTR(string));
            extern lua_State* global_L;
            lpObject_t object = OBJ_LoadDocument(global_L, doc);
            PROP_SetValue(property, &object);
          }
        }
//        struct file *file = FS_LoadFile(string);
//        if (file->size > 4 && *(int*)file->data == MAKE_FOURCC('<','s','v','g')) {
//        }
      } else xmlWith(xmlDoc, doc, FS_LoadXML(string), xmlFreeDoc) {
        extern lua_State* global_L;
        lpObject_t object = OBJ_LoadDocument(global_L, doc);
        PROP_SetValue(property, &object);
      }
      break;
    case kDataTypeGroup:
      FOR_LOOP(i, pdesc->NumComponents) {
        lpcPropertyDesc_t inner = &pdesc[i + 1];
        uint32_t offset = inner->Offset - pdesc->Offset;
        void* destptr = ((byte_t*)dest) + offset;
        SkipSpace(string);
        if (!*(string = PDESC_Parse(hobj, inner, property, string, destptr)))
          break;
      }
      break;
    default:
      assert(!"Unknown type in property");
      break;
  }
  return string;
}
