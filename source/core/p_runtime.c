#include "core_local.h"

#define tok_op(name)                                                           \
  bool_t op_##name(struct token* token,                                        \
                   struct vm_register* regs,                                   \
                   lpObject_t object,                                      \
                   struct vm_register* output)

#define MAX_PROG_REGS 16
#define evaluate op_evaluate

static bool_t
show_error(void)
{
  return FALSE;
}

bool_t
op_evaluate(struct token*,
            struct vm_register*,
            lpObject_t,
            struct vm_register*);

static eDataType_t
InitOutput(struct vm_register* output, eDataType_t type, uint32_t size)
{
  switch (type) {
    case kDataTypeVector2D:
    case kDataTypeVector3D:
    case kDataTypeVector4D:
    case kDataTypeMatrix2D:
    case kDataTypeMatrix3D:
    case kDataTypeTransform2D:
    case kDataTypeTransform3D:
      output->type = kDataTypeFloat;
      break;
    default:
      output->type = type;
      break;
  }
//  output->type = type;
  output->size = size;
  return type;
}

static bool_t
getargint(struct vm_register* r)
{
  bool_t integer = TRUE;
  FOR_LOOP(j, TOKEN_MAX_ARGS)
  {
    integer &= !r[j].type || r[j].type < kDataTypeFloat;
  }
  return integer;
}

static void
getargtype(struct vm_register* r, struct vm_register* o)
{
  FOR_LOOP(j, TOKEN_MAX_ARGS)
  {
    o->type = MAX(o->type, r[j].type);
    o->size = MAX(o->size, r[j].size);
  }
  if (o->type == kDataTypeFloat) {
    o->type = (getargint(r) ? kDataTypeInt : kDataTypeFloat);
  }
}

static xmlNodePtr
findnode(xmlNodePtr node, xmlChar const* lookup)
{
  xmlForEach(child, node)
  {
    xmlWith(xmlChar, name, xmlGetProp(child, XMLSTR("name")), xmlFree)
    {
      if (!xmlStrcmp(name, lookup)) {
        xmlFree(name);
        return child;
      }
    }
  }
  return NULL;
}

static xmlNodePtr
navigatenode(xmlNodePtr data, xmlChar const* sep)
{
  while (data && sep++) {
    xmlChar const* next = xmlStrchr(sep, '/');
    size_t len = next ? (size_t)(next - sep) : xmlStrlen(sep);
    xmlChar name[sizeof(shortStr_t)] = { 0 };
    memcpy(name, sep, MIN(len, sizeof(name) - 1));
    if (!(data = findnode(data, name)))
      return NULL;
    sep = next;
    next = next ? xmlStrchr(++next, '/') : NULL;
  }
  return data;
}

static xmlNodePtr
getservice(lpObject_t hobj, lpcString_t name)
{
  lua_State* L = core.L;
  lua_getfield(L, LUA_REGISTRYINDEX, name);
  if (!lua_isnil(L, -1)) {
    xmlNodePtr service = lua_touserdata(L, -1);
    lua_pop(L, -1);
    return service;
  } else {
    lua_pop(L, -1);
    return NULL;
  }
}

#define CALL(NAME) \
if (!strcmp(token->text, #NAME) && t->type) \
return (token->cache.func=op_##NAME)(token, t, object, output);

#define CALL_XYZ(NAME, FUNC, COMP) \
if (!strcmp(token->text, #NAME) && t->type) {\
token->cache.component = COMP; \
return (token->cache.func=op_##FUNC)(token, t, object, output); \
}

#define OPERATOR(NAME, OPER) \
tok_op(NAME) { \
  getargtype(regs, output); \
  FOR_LOOP(i, output->size / sizeof(float)) \
{ \
  float v[TOKEN_MAX_ARGS]; \
  FOR_LOOP(j, TOKEN_MAX_ARGS) \
  { \
    v[j] = regs[j].value[regs[j].type == kDataTypeFloat ? 0 : i]; \
  } \
    output->value[i] = output->type == kDataTypeInt ? roundf(OPER) : (OPER); \
  } \
  return TRUE; \
}

tok_op(identifier)
{
  memcpy(output, &regs[token->reg], sizeof(*output));
  return TRUE;
}

tok_op(assign)
{
  assert(token->args[0] && token->args[1]);
  if (!evaluate(token->args[1], regs, object, output)) {
    return show_error();
  }
  memcpy(&regs[token->args[0]->reg], output, sizeof(*output));
  return TRUE;
}

tok_op(CONCAT) {
  strncpy((LPSTR)output->value, (lpcString_t)regs[0].value, sizeof(output->value));
  strncat((LPSTR)output->value, (lpcString_t)regs[1].value, sizeof(output->value));
  InitOutput(output, kDataTypeFixed, MAX_PROPERTY_STRING);
  return TRUE;
}
tok_op(STRING) {
  InitOutput(output, kDataTypeFixed, MAX_PROPERTY_STRING);
  switch (regs->type) {
    case kDataTypeBool:
    case kDataTypeFloat:
    case kDataTypeInt:
      snprintf((LPSTR)output->value, sizeof(output->value), "%g", regs->value[0]);
      return TRUE;
    case kDataTypeFixed:
      strncpy((LPSTR)output->value, (lpcString_t)regs->value, sizeof(output->value));
      return TRUE;
    default:
      assert(0);
      return FALSE;
  }
}
tok_op(FLOAT) {
  InitOutput(output, kDataTypeFloat, sizeof(float));
  switch (regs->type) {
    case kDataTypeBool:
    case kDataTypeFloat:
    case kDataTypeInt:
      output->value[0] = regs->value[0];
      return TRUE;
    case kDataTypeFixed:
      output->value[0] = atof((lpcString_t)regs->value);
      return TRUE;
    default:
      assert(0);
      return FALSE;
  }
}
tok_op(INT) {
  InitOutput(output, kDataTypeInt, sizeof(uint32_t));
  switch (regs->type) {
    case kDataTypeBool:
    case kDataTypeFloat:
    case kDataTypeInt:
      output->value[0] = round(regs->value[0]);
      return TRUE;
    case kDataTypeFixed:
      output->value[0] = atoi((lpcString_t)regs->value);
      return TRUE;
    default:
      assert(0);
      return FALSE;
  }
}
tok_op(CREATEROTATION_XYZ) {
  if (regs->type < kDataTypeFixed) {
    memset(output->value, 0, sizeof(output->value));
    InitOutput(output, kDataTypeFloat, sizeof(struct vec3));
    output->value[token->cache.component] = regs->value[0];
    return TRUE;
  } else {
    return FALSE;
  }
}
tok_op(EXTRACT_XYZ) {
  if (regs->type == kDataTypeFloat && regs->size == sizeof(struct vec3)) {
    InitOutput(output, kDataTypeFloat, sizeof(float));
    output->value[0] = regs->value[token->cache.component];
    return TRUE;
  } else {
    return FALSE;
  }
}
tok_op(MAKE_VECTOR) {
  InitOutput(output, kDataTypeFloat, sizeof(float) * token->cache.component);
  FOR_LOOP(i, token->cache.component) {
    if (regs[i].type >= kDataTypeFixed) {
      return FALSE;
    } else {
      output->value[i] = regs[i].value[0];
    }
  }
  return TRUE;
}
tok_op(COLOR4) {
  if (regs[0].type < kDataTypeFixed &&
      regs[1].type < kDataTypeFixed &&
      regs[2].type < kDataTypeFixed &&
      regs[3].type < kDataTypeFixed)
  {
    InitOutput(output, kDataTypeColor, sizeof(struct color));
    output->value[0] = regs[0].value[0];
    output->value[1] = regs[1].value[0];
    output->value[2] = regs[2].value[0];
    output->value[3] = regs[3].value[0];
    return TRUE;
  } else {
    return FALSE;
  }
}
tok_op(ANIMATE) {
  if (regs[1].type == kDataTypeFixed && regs[0].type < kDataTypeFixed) {
    handle_t Animation_Register(xmlDocPtr);
    float animation_evaluate(struct curve*, float, int);
    // handle_t anim = Animation_Register((lpcString_t )regs[1].value);
    lpcString_t path = NULL;
    if (token->args[1]->type == tok_string) {
      path = token->args[1]->text;
    } else {
      path = (lpcString_t)regs[1].value;
    }
    if (!token->cache.animation) {
      token->cache.animation = ZeroAlloc(sizeof(struct curve));
      struct _xmlDoc* (*LoadXmlDoc)(lpcString_t) = token->userdata;
      if (!token->userdata) {
        Con_Error("No userdata set in ANIMATE token");
        return FALSE;
      }
      xmlWith(xmlDoc, doc, LoadXmlDoc(path), xmlFreeDoc) {
        SafeSet(token->cache.animation, Animation_Register(doc), free);
      }
    }
    InitOutput(output, kDataTypeFloat, sizeof(float));
    output->value[0] = animation_evaluate(token->cache.animation, regs[0].value[0], 0);
    return TRUE;
  } else {
    return FALSE;
  }
}

OPERATOR(ADD, v[0] + v[1]);
OPERATOR(MUL, v[0] * v[1]);
OPERATOR(SUB, v[0] - v[1]);
OPERATOR(DIV, v[0] / v[1]);
OPERATOR(EQUAL, fabs(v[0] - v[1]) < 0.0001f);
OPERATOR(UNM, -v[0]);
OPERATOR(STEP, v[0] <= v[1]);
OPERATOR(CLAMP, fmax(v[0], fmin(v[1], v[2])));
OPERATOR(LINEARSTEP, fmax(0, fmin(1, (v[2] - v[0]) / (v[1] - v[0]))));
OPERATOR(MIX, (v[1] - v[0]) * v[2] + v[0]);
OPERATOR(SCALE, fmax(0, fmin(1, (v[2] - v[0]) / (v[1] - v[0]))));
OPERATOR(BOOL, v[0] != 0 ? 1 : 0);
OPERATOR(AND, v[0] != 0 && v[1] != 0);
OPERATOR(OR, v[0] != 0 || v[1] != 0);
OPERATOR(ABS, fabs(v[0]));
OPERATOR(CEIL, ceilf(v[0]));
OPERATOR(FLOOR, floorf(v[0]));
OPERATOR(ROUND, roundf(v[0]));
OPERATOR(SQRT, sqrtf(v[0]));
OPERATOR(MAX, fmax(v[0], v[1]));
OPERATOR(MIN, fmin(v[0], v[1]));
OPERATOR(MOD, fmodf(v[0], v[1]));
OPERATOR(REM, fmodf(v[0], v[1]));
OPERATOR(POW, powf(v[0], v[1]));

tok_op(call)
{
  struct vm_register t[TOKEN_MAX_ARGS] = { 0 };
  FOR_LOOP(j, TOKEN_MAX_ARGS)
  {
    if (token->args[j] && !evaluate(token->args[j], regs, object, t + j)) {
      return show_error();
    }
  }
  if (token->cache.func) {
    return token->cache.func(token, t, object, output);
  }
  
  if (!strcmp(token->text, "ADD") && t[0].type == kDataTypeFixed && t[1].type == kDataTypeFixed) {
    return (token->cache.func=op_CONCAT)(token, t, object, output);
  }
  CALL(ADD);
  CALL(MUL);
  CALL(SUB);
  CALL(DIV);
  CALL(EQUAL);
  CALL(UNM);
  CALL(STEP);
  CALL(CLAMP);
  CALL(LINEARSTEP);
  CALL(MIX);
  CALL(SCALE);
  CALL(BOOL);
  CALL(AND);
  CALL(OR);
  CALL(ABS);
  CALL(CEIL);
  CALL(FLOOR);
  CALL(ROUND);
  CALL(SQRT);
  CALL(MAX);
  CALL(MIN);
  CALL(MOD);
  CALL(REM);
  CALL(POW);
  CALL(STRING);
  CALL(FLOAT);
  CALL(INT);
  CALL(COLOR4);
  CALL(ANIMATE);
  CALL_XYZ(VECTOR2, MAKE_VECTOR, 2);
  CALL_XYZ(VECTOR3, MAKE_VECTOR, 3);
  CALL_XYZ(VECTOR4, MAKE_VECTOR, 4);
  CALL_XYZ(CREATEROTATION, MAKE_VECTOR, 3);
  CALL_XYZ(CREATEROTATIONX, CREATEROTATION_XYZ, 0);
  CALL_XYZ(CREATEROTATIONY, CREATEROTATION_XYZ, 1);
  CALL_XYZ(CREATEROTATIONZ, CREATEROTATION_XYZ, 2);
  CALL_XYZ(VECTOR3X, EXTRACT_XYZ, 0);
  CALL_XYZ(VECTOR3Y, EXTRACT_XYZ, 1);
  CALL_XYZ(VECTOR3Z, EXTRACT_XYZ, 2);
  CALL_XYZ(EXTRACTEULERX, EXTRACT_XYZ, 0);
  CALL_XYZ(EXTRACTEULERY, EXTRACT_XYZ, 1);
  CALL_XYZ(EXTRACTEULERZ, EXTRACT_XYZ, 2);

  Con_Error("Unsupported operation %s\n", token->text);

  assert(0);

  return show_error();
}

LPSTR
ftostr(LPSTR buffer, size_t buffer_size, float value)
{
  // Use snprintf to safely format the float into the buffer with %g.
  int written = snprintf(buffer, buffer_size, "%g", value);
  // If the value was written successfully, return the end of the string.
  if (written > 0 && (size_t)written < buffer_size) {
    return buffer + written;
  }
  // If there was an error or the buffer was too small, return NULL.
  return NULL;
}

static bool_t
PrintToProperty(lpProperty_t prop, struct vm_register* r)
{
  char dest[MAX_PROPERTY_STRING];
  if (r->type == kDataTypeNone) {
    r->type = kDataTypeFixed;
  }
  switch (r->type) {
    case kDataTypeBool:
    case kDataTypeInt:
      snprintf(dest, MAX_PROPERTY_STRING, "%.0g", r->value[0]);
      PROP_SetValue(prop, dest);
      return TRUE;
    case kDataTypeFloat:
      snprintf(dest, MAX_PROPERTY_STRING, "%g", r->value[0]);
      PROP_SetValue(prop, dest);
      return TRUE;
    case kDataTypeFixed:
    case kDataTypeLongString:
      PROP_SetValue(prop, r->value);
      return TRUE;
      //	case kDataTypeGroup: {
      //		LPSTR  d=dest;
      //		uint32_t i=0;
      //		FOR_RLE(prop->userdata) {
      //			switch (*s) {
      //			case 'f':
      //				if (d != dest) *(d++) = ' ';
      //				d = ftostr(d, MAX_PROPERTY_STRING,
      // r->value[i++]); 				break;
      // default: 				assert(!"Unsupported RLE in
      // PrintToProperty()"); 				break;
      //			}
      //		}
      //		PROP_SetValue(prop, dest);
      //		return TRUE;
      //	}
    default:
      //		assert(r->type == PROP_GetType(prop));
      return show_error();
  }
}

static inline bool_t DATA_IsVector(eDataType_t type) {
  switch (type) {
    case kDataTypeFloat:
    case kDataTypeVector2D:
    case kDataTypeVector3D:
    case kDataTypeVector4D:
    case kDataTypeMatrix2D:
    case kDataTypeMatrix3D:
    case kDataTypeTransform2D:
    case kDataTypeTransform3D:
      return TRUE;
    default:
      return FALSE;
  }
}

bool_t
PROP_Import(lpProperty_t prop,
            enum property_attribute attr,
            struct vm_register* r)
{
  eDataType_t type = PROP_GetType(prop);
  if (!attr) {
    if (PROP_GetType(prop) == kDataTypeNone) {
      PROP_SetTypeSize(prop, r->type, r->size);
    }
    if (PROP_GetType(prop) == kDataTypeFixed) {
      return PrintToProperty(prop, r);
    } else if (PROP_GetType(prop) == kDataTypeEnum && r->type == kDataTypeFixed) {
      PROP_SetValue(prop, &(int){
        strlistidx((lpcString_t)r->value, PROP_GetUserData(prop), NULL)
      });
      return TRUE;
    } else if (PROP_GetType(prop) == kDataTypeBool && r->type == kDataTypeFloat) {
      PROP_SetValue(prop, &(int){ *r->value > 0 });
      return TRUE;
    } else if (PROP_GetType(prop) < kDataTypeFloat && r->type < kDataTypeFixed) {
      PROP_SetValue(prop, &(int){ *r->value });
      return TRUE;
    } else if (PROP_GetType(prop) == kDataTypeFloat && r->type < kDataTypeFixed) {
      PROP_SetValue(prop, r->value);
      return TRUE;
    } else if (PROP_GetType(prop) == kDataTypeFloat && r->type == kDataTypeFixed) {
      assert(PROP_GetSize(prop) == sizeof(float));
      PROP_SetValue(prop, &(float){ atof((lpcString_t)r->value) });
      return TRUE;
//    } else if (PROP_GetType(prop) == T_HANDLE && r->type == kDataTypeFixed) {
//      int type = GetPropertyHandleType(PROP_GetUserData(prop));
//      handle_t handle = HACK_LoadResource(type, (lpcString_t)r->value);
//      PROP_SetValue(prop, &handle);
//      return TRUE;
    } else if (PROP_GetType(prop) == kDataTypeGroup && PROP_GetSize(prop) == r->size) {
      PROP_SetValue(prop, r->value);
      return TRUE;
    } else if (r->type == kDataTypeFloat && DATA_IsVector(PROP_GetType(prop))) {
      PROP_SetValue(prop, r->value);
      return TRUE;
    } else if (PROP_GetType(prop) == kDataTypeGroup) {
      lpcPropertyDesc_t pd = PROP_GetDesc(prop);
      FOR_LOOP(i, pd->NumComponents) {
        eDataType_t type = (pd+1+i)->Flags&T_TYPEMASK;
        if (!DATA_IsVector(type) && type != kDataTypeGroup)
          return FALSE;
      }
      PROP_SetValue(prop, r->value);
      return TRUE;
    } else {
      assert(r->type == PROP_GetType(prop));
      switch (PROP_GetType(prop)) {
        case kDataTypeInt:
        case kDataTypeBool:
          PROP_SetValue(prop, &(int){ *r->value });
          return TRUE;
        default:
          PROP_SetValue(prop, r->value);
          return TRUE;
      }
    }
  } else if (type == kDataTypeColor) {
    struct color color = *(struct color const*)PROP_GetValue(prop);
    switch ((uint32_t)attr) {
      case ATTR_COLOR_R:
        assert(r->type < kDataTypeFixed);
        color.r = r->value[0];
        PROP_SetValue(prop, &color);
        return TRUE;
      case ATTR_COLOR_G:
        assert(r->type < kDataTypeFixed);
        color.g = r->value[0];
        PROP_SetValue(prop, &color);
        return TRUE;
      case ATTR_COLOR_B:
        assert(r->type < kDataTypeFixed);
        color.b = r->value[0];
        PROP_SetValue(prop, &color);
        return TRUE;
      case ATTR_COLOR_A:
        assert(r->type < kDataTypeFixed);
        color.a = r->value[0];
        PROP_SetValue(prop, &color);
        return TRUE;
      default:
        Con_Error("Unsupported attribute %d for struct color", attr);
        return show_error();
    }
  } else if (type >= kDataTypeFloat && type <= kDataTypeVector4D) {
    struct vec4 vector = *(struct vec4 const*)PROP_GetValue(prop);
    switch ((uint32_t)attr) {
      case ATTR_VECTOR_X:
        assert(r->type < kDataTypeFixed);
        assert(PROP_GetSize(prop) >= sizeof(struct vec2));
        vector.x = r->value[0];
        PROP_SetValue(prop, &vector);
        return TRUE;
      case ATTR_VECTOR_Y:
        assert(r->type < kDataTypeFixed);
        assert(PROP_GetSize(prop) >= sizeof(struct vec2));
        vector.y = r->value[0];
        PROP_SetValue(prop, &vector);
        return TRUE;
      case ATTR_VECTOR_Z:
        assert(r->type < kDataTypeFixed);
        assert(PROP_GetSize(prop) >= sizeof(struct vec3));
        vector.z = r->value[0];
        PROP_SetValue(prop, &vector);
        return TRUE;
      case ATTR_VECTOR_W:
        assert(r->type < kDataTypeFixed);
        assert(PROP_GetSize(prop) >= sizeof(struct vec4));
        vector.w = r->value[0];
        PROP_SetValue(prop, &vector);
        return TRUE;
      default:
        Con_Error("Unsupported attribute %d for VECTOR%d",
                   attr,
                   (int)(PROP_GetSize(prop) / sizeof(float)));
        return show_error();
    }
  } else {
    Con_Error("Unsupported attribute %d for property %s of type %d",
               attr,
               PROP_GetName(prop),
               PROP_GetType(prop));
    return show_error();
  }
}

static bool_t
jwPropertyExport(lpProperty_t prop,
                 enum property_attribute attr,
                 struct vm_register* r)
{
  PROP_Update(prop);
  if (PROP_IsNull(prop)) {
    return show_error();
  }
  if (!attr) {
    switch (InitOutput(r, PROP_GetType(prop), PROP_GetSize(prop))) {
      case kDataTypeInt:
      case kDataTypeBool:
        *r->value = *(int*)PROP_GetValue(prop);
        return TRUE;
      case kDataTypeLongString:
        strncpy((LPSTR)r->value, PROP_GetValue(prop), sizeof(r->value));
        return TRUE;
      default:
        memcpy(r->value, PROP_GetValue(prop), PROP_GetSize(prop));
        return TRUE;
    }
  }
  eDataType_t type = PROP_GetType(prop);
  switch (type) {
    case kDataTypeColor:
      switch ((uint32_t)attr) {
        case ATTR_COLOR_R:
        case ATTR_COLOR_G:
        case ATTR_COLOR_B:
        case ATTR_COLOR_A:
          InitOutput(r, kDataTypeFloat, sizeof(float));
          r->value[0] = ((float const*)PROP_GetValue(prop))[attr-ATTR_COLOR_R];
          return TRUE;
        default:
          Con_Error("Unsupported attribute %d for struct color", attr);
          return show_error();
      }
      break;
    case kDataTypeVector2D:
    case kDataTypeVector3D:
    case kDataTypeVector4D:
      switch ((uint32_t)attr) {
        case ATTR_VECTOR_X:
        case ATTR_VECTOR_Y:
        case ATTR_VECTOR_Z:
        case ATTR_VECTOR_W:
          InitOutput(r, kDataTypeFloat, sizeof(float));
          r->value[0] = ((float*)PROP_GetValue(prop))[attr-ATTR_VECTOR_X];
          return TRUE;
        default:
          Con_Error("Unsupported attribute %d for VECTOR", attr);
          return show_error();
      }
      break;
    default:
      Con_Error("Unsupported attribute %d for property %s", attr, PROP_GetName(prop));
      return show_error();
  }
}

#include <source/UIKit/UIKit.h> // for GetNode(it)

tok_op(argument)
{
  lpProperty_t p = NULL;
  if (token->cache.property) {
    p = token->cache.property;
    goto return_value;
  }
  if (*token->text == '#') {
    lpcString_t eon = strchr(token->text, '/');
    if (!strncmp(token->text, "##Template/", 11)) {
      object = OBJ_GetParent(object);
      for (; object; object = OBJ_GetParent(object)) {
        if (OBJ_GetFlags(object) & OF_TEMPLATE) {
          p = OBJ_FindPropertyByPath(object, ++eon);
          break;
        }
      }
    } else if (!strncmp(token->text, "##Root/", 7)) {
      p = OBJ_FindPropertyByPath(token->rootnode, ++eon);
    } else /*for (; object; object = OBJ_GetParent(object))*/ if (object) {
      uint32_t ident = fnv1a32_range(token->text + 1, eon);
      lpObject_t child = OBJ_FindChildByAlias(object, ident);
      if (child) {
        p = OBJ_FindPropertyByPath(child, ++eon);
      }
//      break;
    }
  } else if (*token->text == '.') {
    p = OBJ_FindPropertyByPath(object, token->text);
  } else if (!strncmp(token->text, "DataContext/", 12)) {
    for (lpObject_t it = object; it; it = OBJ_GetParent(it)) {
      struct Node* node = GetNode(it);
      if (node && node->DataContext) {
        if ((p = OBJ_FindPropertyByPath(CMP_GetObject(node->DataContext), token->text + 12))) {
          goto return_value;
        }
      }
    }
    memset(output->value, 0, sizeof(*output->value));
    InitOutput(output, kDataTypeFloat, sizeof(float));
    return TRUE;
  } else if (!strcmp(token->text, "DataContext")) {
    for (lpObject_t it = object; it; it = OBJ_GetParent(it)) {
      struct Node* node = GetNode(it);
      if (node && node->DataContext) {
        memcpy(output->value, &node->DataContext, sizeof(void*));
        InitOutput(output, kDataTypeObject, sizeof(handle_t));
        return TRUE;
      }
    }
    return FALSE;
  } else if (strstr(token->text, "Service/")) {
    shortStr_t name = { 0 };
    lpcString_t sep = strchr(token->text, '/');
    strncpy(name, token->text, MIN(sep - token->text, sizeof(name)));
    xmlNodePtr data = navigatenode(getservice(object, name), BAD_CAST sep);
    if (data) {
#define BIND_INT_XML(TYPE)                                                     \
  case type_##TYPE:                                                            \
    output->size = sizeof(TYPE##_t);                                           \
    output->value[0] = *(TYPE##_t*)&data->_private;                            \
    output->type = kDataTypeInt;                                                      \
    return TRUE;

      switch (data->extra) {
        case type_bool:
          output->size = 1;
          output->value[0] = *(LPSTR)&data->_private;
          output->type = kDataTypeBool;
          return TRUE;
          BIND_INT_XML(int8)
          BIND_INT_XML(uint8)
          BIND_INT_XML(int16)
          BIND_INT_XML(uint16)
          BIND_INT_XML(int32)
          BIND_INT_XML(uint32)
          BIND_INT_XML(int64)
          BIND_INT_XML(uint64)
        default:
          Con_Error("Unknwon type %d in XML binding", data->extra);
          break;
      }
    }
  } else {
    lpcString_t innerpath = NULL;
    lpObject_t prefab = OBJ_FindKnownPrefab(token->text, &innerpath);
    if (prefab && strchr(innerpath, '/')) {
      path_t path = { 0 };
      sprintf(path, ".%s", strchr(innerpath, '/'));
      p = OBJ_FindPropertyByPath(prefab, path);
    }
  }
return_value:
  if (p) {
    token->cache.property = p;
    return jwPropertyExport(p, token->attr, output);
  } else {
//    Con_Error("Can't find property \"%s\"", token->text);
    memset(output, 0, sizeof(*output));
    return show_error();
  }
}
tok_op(constant)
{
  if (strchr(token->text, '.')) {
    InitOutput(output, kDataTypeFloat, sizeof(float));
  } else {
    InitOutput(output, kDataTypeInt, sizeof(uint32_t));
  }
  *((float*)output->value) = atof(token->text);
  return TRUE;
}
tok_op(string)
{
  InitOutput(output, kDataTypeFixed, MAX_PROPERTY_STRING);
  strncpy((LPSTR)output->value, token->text, MAX_PROPERTY_STRING);
  return TRUE;
}

typedef bool_t (*op_t)(struct token*,
                       struct vm_register*,
                       lpObject_t,
                       struct vm_register*);

static op_t ops[] = {
  op_identifier, op_assign, op_call, op_argument, op_constant, op_string,
};

bool_t
op_evaluate(struct token* token,
            struct vm_register* regs,
            lpObject_t object,
            struct vm_register* output)
{
  if (!token)
    return show_error();
  assert(token->type < tok_count);
  memset(output, 0, sizeof(*output));
  return ops[token->type](token, regs, object, output);
}

bool_t
OBJ_RunProgram(lpObject_t object,
               struct token* token,
               struct vm_register* output)
{
  struct vm_register vm[MAX_PROG_REGS];
  FOR_EACH_LIST(struct token, tok, token)
  {
    if (!op_evaluate(tok, vm, object, output)) {
      return show_error();
    }
  }
  return TRUE;
}
