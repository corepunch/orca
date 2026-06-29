#include <source/core/core_local.h>

extern struct Object *FS_LoadObject(lpcString_t path);

#define tok_op(name)                                                           \
  bool_t op_##name(struct token* token,                                        \
                   struct vm_register* regs,                                   \
                   struct Object *object,                                      \
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
            struct Object *,
            struct vm_register*);

/* --------------------------------------------------------------------------
 * Circular string pool for VM register strings.
 *
 * String-typed vm_registers store a (const char *) pointer into this pool
 * rather than inline bytes.  The pool is large enough that strings from
 * previous evaluations remain valid for the duration of a single frame.
 * -------------------------------------------------------------------------- */
#define VM_STRPOOL_SIZE 0x10000
static char     s_strpool[VM_STRPOOL_SIZE];
static uint32_t s_strpool_head = 0;

static const char *
vm_strtmp(const char *str)
{
  if (!str) str = "";
  /* Clamp to leave room for the null terminator without reading past str. */
  size_t len = strlen(str);
  if (len >= VM_STRPOOL_SIZE) len = VM_STRPOOL_SIZE - 1;
  size_t total = len + 1;
  if (s_strpool_head + total > VM_STRPOOL_SIZE) {
    s_strpool_head = 0;   /* wrap around */
  }
  char *dest = s_strpool + s_strpool_head;
  memcpy(dest, str, len);
  dest[len] = '\0';
  s_strpool_head += (uint32_t)total;
  return dest;
}

/* Store / retrieve a const char * in the float[] value array. */
#define VM_REG_SET_STR(r, s) \
  do { const char *_p = (s); memcpy((r)->value, &_p, sizeof(_p)); } while (0)
#define VM_REG_STR(r)  (*(const char *const *)(r)->value)

static eDataType_t
InitOutput(struct vm_register* output, eDataType_t type, uint32_t size)
{
  switch (type) {
    case kDataTypeStruct:
    case kDataTypeColor:
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
    WITH(xmlChar, name, xmlGetProp(child, XMLSTR("name")), xmlFree)
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
getservice(struct Object *hobj, lpcString_t name)
{
#if SERVICE_ENABLED
  (void)name;
  Con_Error("SERVICE_ENABLED requires an explicit Lua state provider");
#else
  (void)name;
#endif
  (void)hobj;
  return NULL;
}

static struct Object *
_BindingTemplateOrRoot(struct Object *object)
{
  struct Object *root = object;
  for (struct Object *it = object; it; it = OBJ_GetParent(it)) {
    if (OBJ_GetFlags(it) & OF_TEMPLATE) {
      return it;
    }
    root = it;
  }
  return root;
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
  char tmp[MAX_PROPERTY_STRING];
  snprintf(tmp, sizeof(tmp), "%s%s", VM_REG_STR(&regs[0]), VM_REG_STR(&regs[1]));
  InitOutput(output, kDataTypeString, sizeof(const char *));
  VM_REG_SET_STR(output, vm_strtmp(tmp));
  return TRUE;
}
tok_op(STRING) {
  char tmp[MAX_PROPERTY_STRING];
  InitOutput(output, kDataTypeString, sizeof(const char *));
  switch (regs->type) {
    case kDataTypeBool:
    case kDataTypeFloat:
    case kDataTypeInt:
      snprintf(tmp, sizeof(tmp), "%g", regs->value[0]);
      VM_REG_SET_STR(output, vm_strtmp(tmp));
      return TRUE;
    case kDataTypeString:
      VM_REG_SET_STR(output, VM_REG_STR(regs));  /* pointer copy — no re-allocation */
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
    case kDataTypeString:
      output->value[0] = (float)atof(VM_REG_STR(regs));
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
    case kDataTypeString:
      output->value[0] = (float)atoi(VM_REG_STR(regs));
      return TRUE;
    default:
      assert(0);
      return FALSE;
  }
}
tok_op(CREATEROTATION_XYZ) {
  if (regs->type < kDataTypeString) {
    memset(output->value, 0, sizeof(output->value));
    InitOutput(output, kDataTypeFloat, sizeof(struct vec3));
    output->value[token->cache.component] = regs->value[0];
    return TRUE;
  } else {
    return FALSE;
  }
}
static inline bool_t DATA_IsVector(eDataType_t type) {
  return type == kDataTypeFloat || type == kDataTypeStruct || type == kDataTypeColor;
}
tok_op(EXTRACT_XYZ) {
  if (DATA_IsVector(regs->type) &&
      regs->size >= sizeof(float) * (uint32_t)(token->cache.component + 1)) {
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
    if (regs[i].type >= kDataTypeString) {
      return FALSE;
    } else {
      output->value[i] = regs[i].value[0];
    }
  }
  return TRUE;
}
tok_op(COLOR4) {
  if (regs[0].type < kDataTypeString &&
      regs[1].type < kDataTypeString &&
      regs[2].type < kDataTypeString &&
      regs[3].type < kDataTypeString)
  {
    InitOutput(output, kDataTypeFloat, sizeof(struct color));
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
  return FALSE;
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

tok_op(IF) {
  /* regs[0] = condition, regs[1] = true-value, regs[2] = false-value */
  bool_t cond;
  if (regs[0].type == kDataTypeString) {
    const char *s = VM_REG_STR(&regs[0]);
    cond = s && *s != '\0';
  } else {
    cond = regs[0].value[0] != 0.0f;
  }
  struct vm_register *result = cond ? &regs[1] : &regs[2];
  memcpy(output, result, sizeof(*output));
  return TRUE;
}

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
  
  if (!strcmp(token->text, "ADD") && t[0].type == kDataTypeString && t[1].type == kDataTypeString) {
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
  CALL(IF);
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
  CALL_XYZ(COLORR, EXTRACT_XYZ, 0);
  CALL_XYZ(COLORG, EXTRACT_XYZ, 1);
  CALL_XYZ(COLORB, EXTRACT_XYZ, 2);
  CALL_XYZ(COLORA, EXTRACT_XYZ, 3);
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

static void set_string(struct Property *prop, char const* str) {
  char** value = (char**)PROP_GetValue(prop);
  if (!*value || strcmp(*value, str)) {
    PROP_SetValue(prop, &str);
  }
}

static bool_t
PrintToProperty(struct Property *prop, struct vm_register* r)
{
  char dest[MAX_PROPERTY_STRING];
  switch (r->type) {
    case kDataTypeBool:
    case kDataTypeInt:
      snprintf(dest, MAX_PROPERTY_STRING, "%.0g", r->value[0]);
      set_string(prop, dest);
      return TRUE;
    case kDataTypeFloat:
      snprintf(dest, MAX_PROPERTY_STRING, "%g", r->value[0]);
      set_string(prop, dest);
      return TRUE;
    case kDataTypeString:
      set_string(prop, VM_REG_STR(r));
      return TRUE;
    default:
      return show_error();
  }
}

bool_t
PROP_Import(struct Property *prop,
            struct vm_register* r)
{
  switch (PROP_GetType(prop)) {
    case kDataTypeString:
      return PrintToProperty(prop, r);

    case kDataTypeEnum:
      if (r->type == kDataTypeString) {
        lpcString_t const* enum_values = PROP_GetDesc(prop)->EnumValues;
        int idx = 0;
        while (enum_values && enum_values[idx] && strcasecmp(enum_values[idx], VM_REG_STR(r)) != 0)
          idx++;
        PROP_SetValue(prop, &idx);
        return TRUE;
      }
      break;

    case kDataTypeBool:
      switch (r->type) {
        case kDataTypeFloat:
          PROP_SetValue(prop, &(float){ *r->value > 0 });
          return TRUE;
        case kDataTypeInt:
        case kDataTypeBool:
        case kDataTypeEnum:
          PROP_SetValue(prop, &(int){ *r->value > 0 });
          return TRUE;
        case kDataTypeString: {
          const char *boolstr = *r->value > 0 ? "true" : "false";
          PROP_SetValue(prop, &boolstr);
          return TRUE;
        }
        default:
          return FALSE;
      }

    case kDataTypeInt:
      switch (r->type) {
        case kDataTypeInt:
        case kDataTypeBool:
        case kDataTypeEnum:
        case kDataTypeFloat:
          PROP_SetValue(prop, &(int){ *r->value });
          return TRUE;
        default:
          return FALSE;
      }

    case kDataTypeFloat:
      switch (r->type) {
        case kDataTypeString:
          assert(PROP_GetSize(prop) == sizeof(float));
          PROP_SetValue(prop, &(float){ (float)atof(VM_REG_STR(r)) });
          return TRUE;
        case kDataTypeInt:
        case kDataTypeBool:
        case kDataTypeEnum:
          PROP_SetValue(prop, &(int){ *r->value });
          return TRUE;
        case kDataTypeFloat:
          PROP_SetValue(prop, r->value);
          return TRUE;
        default:
          return FALSE;
      }

    case kDataTypeColor:
      if (DATA_IsVector(r->type)) {
        PROP_SetValue(prop, r->value);
        return TRUE;
      }
      if (PROP_GetSize(prop) == r->size) {
        PROP_SetValue(prop, r->value);
        return TRUE;
      }
      if (r->type == kDataTypeString) {
        struct color clr = COLOR_Parse(VM_REG_STR(r));
        PROP_SetValue(prop, &clr);
        return TRUE;
      }
      Con_Error("Binding import failed: cannot assign register type %d (size %u) to property %s of type %d (size %u)",
                r->type,
                r->size,
                PROP_GetName(prop),
                PROP_GetType(prop),
                PROP_GetSize(prop));
      return FALSE;

    case kDataTypeStruct:
      if (DATA_IsVector(r->type)) {
        PROP_SetValue(prop, r->value);
        return TRUE;
      }
      if (PROP_GetSize(prop) == r->size) {
        PROP_SetValue(prop, r->value);
        return TRUE;
      }
      Con_Error("Binding import failed: cannot assign register type %d (size %u) to property %s of type %d (size %u)",
                r->type,
                r->size,
                PROP_GetName(prop),
                PROP_GetType(prop),
                PROP_GetSize(prop));
      return FALSE;

    case kDataTypeObject:
      if (r->type == kDataTypeString) {
        struct Object *loaded = FS_LoadObject(VM_REG_STR(r));
        if (loaded) {
          PROP_SetValue(prop, &loaded);
          return TRUE;
        }
      }
      if (PROP_GetSize(prop) == r->size) {
        PROP_SetValue(prop, r->value);
        return TRUE;
      }
      Con_Error("Binding import failed: cannot assign register type %d to property %s of type %d",
                r->type,
                PROP_GetName(prop),
                PROP_GetType(prop));
      return FALSE;

    default:
      if (r->type != PROP_GetType(prop)) {
        Con_Error("Binding import failed: cannot assign register type %d to property %s of type %d",
                  r->type,
                  PROP_GetName(prop),
                  PROP_GetType(prop));
        return FALSE;
      }
      PROP_SetValue(prop, r->value);
      return TRUE;
  }
  return FALSE;
}

static bool_t
PROP_Export(struct Property *prop,
            struct vm_register* r)
{
  PROP_Update(prop);
  void const *value = PROP_GetValue(prop);
  switch (InitOutput(r, PROP_GetType(prop), PROP_GetSize(prop))) {
    case kDataTypeInt:
    case kDataTypeBool:
      *r->value = value ? *(int*)value : 0;
      return TRUE;
    case kDataTypeString: {
      lpcString_t *str = (lpcString_t*)value;
      InitOutput(r, kDataTypeString, sizeof(const char *));
      VM_REG_SET_STR(r, vm_strtmp(str ? *str : NULL));
      return TRUE;
    }
    default:
      if (value) {
        memcpy(r->value, value, PROP_GetSize(prop));
      } else {
        memset(r->value, 0, PROP_GetSize(prop));
      }
      return TRUE;
  }
}

static bool_t
_BindingPropertyHasEffectiveValue(struct Property *prop)
{
  if (!prop) {
    return FALSE;
  }
  if (!PROP_IsNull(prop)) {
    return TRUE;
  }
  return PROP_GetDesc(prop) && PROP_GetDesc(prop)->IsInherited && PROP_GetValue(prop) != NULL;
}

#include <UIKit/UIKit.h> // for GetNode/GetScreen

static struct Object *
_FindDataContextObject(struct Object *object, void **component)
{
  struct Node *node = GetNode(object);
  if (node && node->DataContext) {
    if (component) {
      *component = node->DataContext;
    }
    return CMP_GetObject(node->DataContext);
  }

  struct Screen *screen = GetScreen(object);
  if (screen && screen->DataContext) {
    if (component) {
      *component = screen->DataContext;
    }
    return CMP_GetObject(screen->DataContext);
  }

  return NULL;
}

tok_op(argument)
{
  struct Property *p = NULL;
  int isDataContextPath = 0;
  if (token->cache.property) {
    p = token->cache.property;
    if (_BindingPropertyHasEffectiveValue(p)) {
      goto return_value;
    }
    token->cache.property = NULL;
    p = NULL;
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
      struct Object *child = OBJ_FindChildByAlias(object, ident);
      if (child) {
        p = OBJ_FindPropertyByPath(child, ++eon);
      }
//      break;
    }
  } else if (*token->text == '.') {
    p = OBJ_FindPropertyByPath(object, token->text);
  } else if (!strncmp(token->text, "DataContext/", 12)) {
    for (struct Object *it = object; it; it = OBJ_GetParent(it)) {
      struct Object *context = _FindDataContextObject(it, NULL);
      if (context) {
        if ((p = OBJ_FindPropertyByPath(context, token->text + 12))) {
          isDataContextPath = 1;
          goto return_value;
        }
      }
    }
    memset(output->value, 0, sizeof(*output->value));
    InitOutput(output, kDataTypeFloat, sizeof(float));
    return TRUE;
  } else if (!strcmp(token->text, "DataContext")) {
    for (struct Object *it = object; it; it = OBJ_GetParent(it)) {
      void *dataContext = NULL;
      if (_FindDataContextObject(it, &dataContext)) {
        memcpy(output->value, &dataContext, sizeof(void*));
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
    /* Default: bare paths first target the bound object. If not found, keep
       compatibility with template/root-scoped bindings. Project property types
       can auto-create empty properties, so an unmodified local property should
       not block a real value on the template root. */
    struct Object *scope = _BindingTemplateOrRoot(object);
    p = OBJ_FindPropertyByPath(object, token->text);
    if (scope && scope != object && (!p || !_BindingPropertyHasEffectiveValue(p))) {
      struct Property *scoped = OBJ_FindPropertyByPath(scope, token->text);
      if (scoped && (_BindingPropertyHasEffectiveValue(scoped) || !p)) {
        p = scoped;
      }
    }
  }
return_value:
  if (p) {
    if (_BindingPropertyHasEffectiveValue(p) && !isDataContextPath) {
      token->cache.property = p;
    }
    return PROP_Export(p, output);
  } else {
    Con_Error("Can't find property \"%s\"", token->text);
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
  InitOutput(output, kDataTypeString, sizeof(const char *));
  VM_REG_SET_STR(output, vm_strtmp(token->text));
  return TRUE;
}

typedef bool_t (*op_t)(struct token*,
                       struct vm_register*,
                       struct Object *,
                       struct vm_register*);

static op_t ops[] = {
  op_identifier, op_assign, op_call, op_argument, op_constant, op_string,
};

bool_t
op_evaluate(struct token* token,
            struct vm_register* regs,
            struct Object *object,
            struct vm_register* output)
{
  if (!token)
    return show_error();
  assert(token->type < tok_count);
  memset(output, 0, sizeof(*output));
  return ops[token->type](token, regs, object, output);
}

bool_t
OBJ_RunProgram(struct Object *object,
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
